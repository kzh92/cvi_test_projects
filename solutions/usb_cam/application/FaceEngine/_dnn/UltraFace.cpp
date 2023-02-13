//
//  UltraFace.cpp
//  UltraFaceTest
//
//  Created by vealocia on 2019/10/17.
//  Copyright © 2019 vealocia. All rights reserved.
//

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#define g_score_threshold_  (0.7f)
#define g_iou_threshold_    (0.3f)

#include <math.h>

#include "UltraFace.hpp"
#include "detect.h"
#include <memory.h>
#include "HAlign.h"
#include "dic_manage.h"
#include "common_types.h"

extern void APP_LOG(const char * format, ...);

#if MODEL_TYPE == RFB_lite
#define NUM_ANCHORS     (885) // (3 * 20 * 12 + 2 * 10 * 6 + 3 * 5 * 3)
#endif

#if MODEL_TYPE == Mn_lite
#define NUM_ANCHORS     (1260) // (3 * 24 * 14 + 2 * 12 * 7 + 3 * 6 * 4 + 2 * 3 * 2)
#endif

#define NUM_ANCHORS_2   (NUM_ANCHORS>>1)

Detect g_Detector;
extern int g_nStopEngine;

float score_threshold;
float iou_threshold;
float center_variance;
float size_variance;

//std::vector<std::vector<float> > priors_Portrait;
//std::vector<std::vector<float> > priors_Landscape;

float* priors_Portrait = 0;
float* priors_Landscape = 0;
FaceInfo *bbox_collection = 0;
int *merged = 0;
int *buf = 0;

int getDetectMenSize()
{
    int nMemSize = 0;
    nMemSize += Detect::dnn_mem_size();
    nMemSize += sizeof(FaceInfo) * NUM_ANCHORS;
    nMemSize += sizeof(int) * NUM_ANCHORS * 2;
    return nMemSize;
}

int initDetectionEngineParam(unsigned char* pMem)
{
    int in_w;
    int in_h;
    int in_w_base;
    int in_h_base;
    float w_h_list[2];
    int w_h_list_base[2];
    float featuremap_size[2][num_featuremap];
    float featuremap_size_base[2][num_featuremap];
    float shrinkage_size[2][num_featuremap];

    score_threshold = g_score_threshold_;
    iou_threshold = g_iou_threshold_;
    in_w = g_DNN_Detection_input_width;
    in_h = g_DNN_Detection_input_height;
    w_h_list[0] = in_w;
    w_h_list[1] = in_h;

    in_w_base = g_DNN_Detection_input_width_base;
    in_h_base = g_DNN_Detection_input_height_base;
    w_h_list_base[0] = in_w_base;
    w_h_list_base[1] = in_h_base;

#if MODEL_TYPE == RFB
    int min_boxes_cnt[num_featuremap] = {3, 2, 2, 3};
    float min_boxes[num_featuremap][3] = {
        { 10.0f, 16.0f, 24.0f },
        { 32.0f, 48.0f, 0.0f },
        { 64.0f, 96.0f, 0.0f },
        { 128.0f, 192.0f, 256.0f }
    };

    float strides[num_featuremap] = { 8.0, 16.0, 32.0, 64.0 };
#endif

#if MODEL_TYPE == RFB_lite
    int min_boxes_cnt[num_featuremap] = {3, 2, 3};
    float min_boxes[num_featuremap][3] = {
        { 10.0f, 16.0f, 24.0f },
        { 32.0f, 48.0f, 0.0f },
        { 64.0f, 96.0f, 128.0f }
        //{ 64.0f, 96.0f}
     };

    float strides[num_featuremap] = { 8.0, 16.0, 32.0 };
#endif

#if MODEL_TYPE == Mn_lite
    int min_boxes_cnt[num_featuremap] = {3, 2, 3, 2};
    float min_boxes[num_featuremap][3] = {
        { 10.0f, 16.0f, 24.0f },
        { 32.0f, 48.0f },
        { 64.0f, 96.0f, 128.0f },
        { 192.0f, 256.0f }
     };

    float strides[num_featuremap] = { 8.0, 16.0, 32.0, 64.0 };
#endif

    center_variance = 0.1;
    size_variance = 0.2;


    for(unsigned int nindex = 0; nindex < 2; nindex ++)
   {
       int size = w_h_list[nindex];
       for (int stride_idx = 0; stride_idx < num_featuremap; stride_idx++)
       {
           float stride = strides[stride_idx];
           featuremap_size[nindex][stride_idx] = ceil(size / stride);
       }

       size = w_h_list_base[nindex];
       for (int stride_idx = 0; stride_idx < num_featuremap; stride_idx++)
       {
           float stride = strides[stride_idx];
           featuremap_size_base[nindex][stride_idx] = ceil(size / stride);
       }
   }

    int nSizeindex = 0;
    for (int nindex = 0; nindex < 2; nindex++)
    {
        int size = w_h_list_base[nindex];
        for (unsigned int k = 0; k < num_featuremap; k++)
        {
            shrinkage_size[nindex][k] = size / featuremap_size_base[nSizeindex][k];
        }
        nSizeindex++;
    }


    priors_Portrait = (float*)my_malloc(sizeof(float) * NUM_ANCHORS * 4 * 2);
    priors_Landscape = priors_Portrait + NUM_ANCHORS * 4;
    int priors_Portrait_cnt = 0;
    int priors_Landscape_cnt = 0;

    /* generate prior anchors */
    for (int index = 0; index < num_featuremap; index++) {
        float scale_w = in_w_base / shrinkage_size[0][index];
        float scale_h = in_h_base / shrinkage_size[1][index];
        for (int j = 0; j < featuremap_size[1][index]; j++) {
            for (int i = 0; i < featuremap_size[0][index]; i++) {
                float x_center = (i + 0.5) / scale_w;
                float y_center = (j + 0.5) / scale_h;

                for (int k_idx = 0; k_idx < min_boxes_cnt[index]; k_idx++) {
                    float k = min_boxes[index][k_idx];
                    float w = k / in_w_base;
                    float h = k / in_h_base;
                    int priors_cnt_4 = priors_Portrait_cnt << 2;
                    priors_Portrait[priors_cnt_4] = clip(x_center, 1);
                    priors_Portrait[priors_cnt_4 + 1] = clip(y_center, 1);
                    priors_Portrait[priors_cnt_4 + 2] = clip(w, 1);
                    priors_Portrait[priors_cnt_4 + 3] = clip(h, 1);
                    priors_Portrait_cnt ++;
                    //priors_Portrait.push_back({clip(x_center, 1), clip(y_center, 1), clip(w, 1), clip(h, 1)});
                }
            }
        }

        for (int j = 0; j < featuremap_size[0][index]; j++)
        {
            for (int i = 0; i < featuremap_size[1][index]; i++)
            {
                float x_center = (i + 0.5) / scale_h;
                float y_center = (j + 0.5) / scale_w;

                for (int k_idx = 0; k_idx < min_boxes_cnt[index]; k_idx++) {
                    float k = min_boxes[index][k_idx];
                    float w = k / in_h_base;
                    float h = k / in_w_base;
                    int priors_cnt_4 = priors_Landscape_cnt << 2;
                    priors_Landscape[priors_cnt_4] = clip(x_center, 1);
                    priors_Landscape[priors_cnt_4 + 1] = clip(y_center, 1);
                    priors_Landscape[priors_cnt_4 + 2] = clip(w, 1);
                    priors_Landscape[priors_cnt_4 + 3] = clip(h, 1);
                    priors_Landscape_cnt++;

                    //priors_Landscape.push_back({ clip(x_center, 1), clip(y_center, 1), clip(w, 1), clip(h, 1) });
                }
            }
        }
    }
    unsigned char* addr = pMem;
    bbox_collection = (FaceInfo*)addr;          addr += sizeof(FaceInfo) * NUM_ANCHORS;
    merged = (int*)addr;                        addr += sizeof(int) * NUM_ANCHORS;
    buf = (int*)addr;                           addr += sizeof(int) * NUM_ANCHORS;

    return 0;
}

int createDetectEngine(unsigned char* pMem)
{
    if(g_Detector.getEngineLoaded())
    {
        return 0;
    }
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Detect))
    {
        return 1;
    }

    int nRet = 0;
    int nDicSize = g_Detector.dnn_dic_size();
    nRet =  g_Detector.dnn_create(g_dic_detect, nDicSize, pMem);
    if(nRet)
    {
        return nRet;
    }

    initDetectionEngineParam(pMem + nDicSize);

    return 0;
}

int releaseDetectEngine()
{
    releaseMachineDic(MachineFlagIndex_DNN_Detect);
    g_Detector.dnn_free();
    if(priors_Portrait)
    {
        my_free(priors_Portrait);
        priors_Portrait = 0;
    }
    return 0;
}

void generateBBox(FaceInfo *bbox_collection, int* pn_cnt, int max_count_collection, float* scores, float* boxes, float score_threshold, int num_anchors, int nPortrait, int image_w, int image_h)
{
    int n_cnt = 0;
    for (int i = 0; i < num_anchors; i++)
    {
        int i_4 = i << 2;
        if (scores[i * 2 + 1] > score_threshold)
        {
            FaceInfo rects;
            float x_center;
            float y_center;
            float w;
            float h;
            if (nPortrait)
            {
                x_center = boxes[i * 4] * center_variance * priors_Portrait[i_4 + 2] + priors_Portrait[i_4];
                y_center = boxes[i * 4 + 1] * center_variance * priors_Portrait[i_4 + 3] + priors_Portrait[i_4 + 1];
                w = exp(boxes[i * 4 + 2] * size_variance) * priors_Portrait[i_4 + 2];
                h = exp(boxes[i * 4 + 3] * size_variance) * priors_Portrait[i_4 + 3];

            }
            else
            {
                x_center = boxes[i * 4] * center_variance * priors_Landscape[i_4 + 2] + priors_Landscape[i_4];
                y_center = boxes[i * 4 + 1] * center_variance * priors_Landscape[i_4 + 3] + priors_Landscape[i_4 + 1];
                w = exp(boxes[i * 4 + 2] * size_variance) * priors_Landscape[i_4 + 2];
                h = exp(boxes[i * 4 + 3] * size_variance) * priors_Landscape[i_4 + 3];
            }
            rects.x1 = (x_center - w / 2.0f) * image_w;
            rects.y1 = (y_center - h / 2.0f) * image_h;
            rects.x2 = (x_center + w / 2.0f) * image_w;
            rects.y2 = (y_center + h / 2.0f) * image_h;
            rects.score = clip(scores[i * 2 + 1], 1);
            //bbox_collection.push_back(rects);
            if(n_cnt < max_count_collection)
            {
                bbox_collection[n_cnt] = rects;
                n_cnt++;
            }
        }
    }
    *pn_cnt = n_cnt;
}

int comp_faceinfo(const void* a, const void* b)
{
    return ((FaceInfo*)a)->score < ((FaceInfo*)b)->score;
}


void nms(FaceInfo* input, int n_input_cnt, FaceInfo* output, int* pn_output_cnt, int n_output_MaxFaceNum, int type)
{
    // std::sort(input.begin(), input.end(), [](const FaceInfo &a, const FaceInfo &b) { return a.score > b.score; });
    qsort(input, n_input_cnt, sizeof(FaceInfo), comp_faceinfo);

    int box_num = n_input_cnt;
    int out_cnt = 0;
    memset(merged, 0, sizeof(int) * box_num);

    for (int i = 0; i < box_num; i++) {

        if (merged[i])
            continue;

        int buf_cnt = 0;
        memset(buf, 0, sizeof(int) * box_num);
        // std::vector<FaceInfo> buf;

        buf[buf_cnt] = i;
        buf_cnt++;
        // buf.push_back(input[i]);
        merged[i] = 1;

        float h0 = input[i].y2 - input[i].y1 + 1;
        float w0 = input[i].x2 - input[i].x1 + 1;

        float area0 = h0 * w0;

        for (int j = i + 1; j < box_num; j++) {
            if (merged[j])
                continue;

            float inner_x0 = input[i].x1 > input[j].x1 ? input[i].x1 : input[j].x1;
            float inner_y0 = input[i].y1 > input[j].y1 ? input[i].y1 : input[j].y1;

            float inner_x1 = input[i].x2 < input[j].x2 ? input[i].x2 : input[j].x2;
            float inner_y1 = input[i].y2 < input[j].y2 ? input[i].y2 : input[j].y2;

            float inner_h = inner_y1 - inner_y0 + 1;
            float inner_w = inner_x1 - inner_x0 + 1;

            if (inner_h <= 0 || inner_w <= 0)
                continue;

            float inner_area = inner_h * inner_w;

            float h1 = input[j].y2 - input[j].y1 + 1;
            float w1 = input[j].x2 - input[j].x1 + 1;

            float area1 = h1 * w1;

            float score;

            score = inner_area / (area0 + area1 - inner_area);

            if (score > iou_threshold) {
                merged[j] = 1;
                buf[buf_cnt] = j;
                buf_cnt++;
                // buf.push_back(input[j]);
            }
        }
        switch (type) {
            case hard_nms: {
                if(out_cnt < n_output_MaxFaceNum)
                {
                    output[out_cnt] = input[buf[0]];
                    out_cnt++;
                }
                // output.push_back(buf[0]);
                break;
            }
            case blending_nms: {
                float total = 0;
                for (int i = 0; i < buf_cnt; i++) {
                    total += exp(input[buf[i]].score);
                }
                FaceInfo rects;
                memset(&rects, 0, sizeof(rects));
                for (int i = 0; i < buf_cnt; i++) {
                    float rate = exp(input[buf[i]].score) / total;
                    rects.x1 += input[buf[i]].x1 * rate;
                    rects.y1 += input[buf[i]].y1 * rate;
                    rects.x2 += input[buf[i]].x2 * rate;
                    rects.y2 += input[buf[i]].y2 * rate;
                    rects.score += input[buf[i]].score * rate;
                }
                if(out_cnt < n_output_MaxFaceNum)
                {
                    output[out_cnt] = rects;
                    out_cnt++;
                }
                //output.push_back(rects);
                break;
            }
            default: {
                //printf("wrong type of nms.");
                exit(-1);
            }
        }
    }

    *pn_output_cnt = out_cnt;
}

int detect(unsigned char* imgBuffer, int imageWidth, int imageHeight, FaceInfo* face_list, int nMaxFaceNum, int* pn_facelist_cnt, unsigned char* pTempBuffer)
{
    if(!g_Detector.getEngineLoaded() || !getDicChecSumChecked(MachineFlagIndex_DNN_Detect))
    {
        return 0;
    }
    float rScale = 1;
    //float startTime = Now();

    CreateShrinkImage_normalize_FixRate(0, pTempBuffer, g_DNN_Detection_input_width, g_DNN_Detection_input_height, &rScale, imgBuffer, imageWidth, imageHeight, 127, 1.0f / 128);
    //printf("CreateShrink Time = %f\n", Now() - startTime);
    //startTime = Now();

    int bufferWidth  = g_DNN_Detection_input_width;
    int bufferHeight  = g_DNN_Detection_input_height;

    int image_w;
    int image_h;

    int nPortrait = 0;
    if (bufferWidth > bufferHeight)
    {
        image_h = g_DNN_Detection_input_width_base;
        image_w = g_DNN_Detection_input_height_base;
    }
    else
    {
        image_h = g_DNN_Detection_input_height_base;
        image_w = g_DNN_Detection_input_width_base;
        nPortrait = 1;
    }

    float* scores_ptr;
    float* boxes_ptr;
    g_Detector.dnn_forward(pTempBuffer, bufferWidth, bufferHeight, &scores_ptr, &boxes_ptr);
    //printf("g_Detector.dnn_forward Time = %f\n", Now() - startTime);

    if (g_nStopEngine == 1)
    {
        return 0;
    }
    int bbox_cnt = 0;
    generateBBox(bbox_collection, &bbox_cnt, NUM_ANCHORS_2, scores_ptr, boxes_ptr, score_threshold, NUM_ANCHORS, nPortrait, image_w, image_h);
    if (g_nStopEngine == 1)
    {
        return 0;
    }

    nms(bbox_collection, bbox_cnt, face_list, pn_facelist_cnt, nMaxFaceNum, blending_nms);

    int nFaceNum =  *pn_facelist_cnt;
    float rScaleX = rScale;
    float rScaleY = rScale;
    int nFaceIndex;
    for (nFaceIndex = 0; nFaceIndex < nFaceNum; nFaceIndex++)
    {
        face_list[nFaceIndex].x1 = face_list[nFaceIndex].x1 * rScaleX;
        face_list[nFaceIndex].x2 = face_list[nFaceIndex].x2 * rScaleX;
        face_list[nFaceIndex].y1 = face_list[nFaceIndex].y1 * rScaleY;
        face_list[nFaceIndex].y2 = face_list[nFaceIndex].y2 * rScaleY;
    }

    return 0;
}

int checkFace(unsigned char* img, int bufferWidth, int bufferHeight)
{
    if(!g_Detector.getEngineLoaded() || !getDicChecSumChecked(MachineFlagIndex_DNN_Detect))
    {
        return 0;
    }

    int nFaceExist = 0;

    int nBufferIndex;
    float* scores_ptr;
    float* boxes_ptr;
    if (g_nStopEngine == 1)
    {
        return 0;
    }
    int cnt = g_Detector.dnn_forward(img, bufferWidth, bufferHeight, &scores_ptr, &boxes_ptr, 1);
    if (g_nStopEngine == 1)
    {
        return 0;
    }
    for (nBufferIndex = 0; nBufferIndex < cnt; nBufferIndex ++)
    {
        if(scores_ptr[nBufferIndex * 2 + 1] > 0.6)
        {
            nFaceExist = 1;
            break;
        }
    }
    return nFaceExist;
}


void refineRect(float* pnRect, int nWidth, int nHeight)
{
    if(pnRect[0] < 0)
    {
        pnRect[0] = 0;
    }

    if(pnRect[1] < 0)
    {
        pnRect[1] = 0;
    }

    if(pnRect[2] < 1)
    {
        pnRect[2] = 1;
    }

    if(pnRect[3] < 1)
    {
        pnRect[3] = 1;
    }

    if(pnRect[0] >= nWidth -  1)
    {
        pnRect[0] = nWidth - 2;
    }

    if(pnRect[1] >= nHeight - 1)
    {
        pnRect[1] = nHeight - 2;
    }

    if(pnRect[2] >= nWidth)
    {
        pnRect[2] = nWidth - 1;
    }

    if(pnRect[3] >= nHeight)
    {
        pnRect[3] = nHeight - 1;
    }
}
