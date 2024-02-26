#include "ds_h264_types.h"
#include <stdio.h>
#include "cvi_venc.h"
#include "cvi_sys.h"

#if (USE_USB_XN_PROTO)

// uint8_t *h26x_buffer_media = (uint8_t *)aos_malloc(MAX_H26X_FRAME_SIZE);
// memset(h26x_buffer_media, 0, MAX_H26X_FRAME_SIZE);

int xndsParseH264CviStream(void* pstStream)
{
    VENC_STREAM_S *pstH26XStream = (VENC_STREAM_S *)pstStream;
    VENC_PACK_S *ppack;
    int buf_len = 0;
    for (unsigned int i = 0; i < pstH26XStream->u32PackCount; ++i)
    {
        ppack = &pstH26XStream->pstPack[i];
        if(buf_len + (ppack->u32Len - ppack->u32Offset) <= MAX_H26X_FRAME_SIZE){
            buf_len += (ppack->u32Len - ppack->u32Offset);
        }
        else{
            //overflow
            return -1;
        }
    }
    return buf_len;
}

int xndsCopyFromH26XStream(unsigned char* pstDstBuf, void* pstStream, int nOffset, int nLength)
{
#if 0
    VENC_STREAM_S *pstH26XStream = (VENC_STREAM_S *)pstStream;
    int buf_len = 0;
    int off;
    int len;
    for (i = 0; i < pstH26XStream->u32PackCount; ++i)
    {
        ppack = &pstH26XStream->pstPack[i];
        if(buf_len + (ppack->u32Len - ppack->u32Offset) <= MAX_H26X_FRAME_SIZE){
            if (buf_len + (ppack->u32Len - ppack->u32Offset) > nOffset)
            {
                if (buf_len >= nOffset)
                {
                    off = 0;
                    len = nLength - (buf_len - nOffset);
                    if (len > 0)
                    {
                        if (len > (ppack->u32Len - ppack->u32Offset))
                            len = (ppack->u32Len - ppack->u32Offset);
                        memcpy(pstDstBuf, ppack->pu8Addr + ppack->u32Offset + off, len);
                    }
                }
                else
                {
                    off = nOffset - buf_len;
                    len = buf_len + (ppack->u32Len - ppack->u32Offset) - nOffset;
                    if (len > 0)
                    {
                        if (len > nLength)
                            len = nLength;
                        memcpy(pstDstBuf, ppack->pu8Addr + ppack->u32Offset + off, len);
                    }
                }
            }
            //memcpy(h26x_buffer_media + buf_len, ppack->pu8Addr + ppack->u32Offset, ppack->u32Len - ppack->u32Offset);
            buf_len += (ppack->u32Len - ppack->u32Offset);
        }
        else{
            //overflow
            return -1;
        }
    }
#endif
    return 0;
}

int xndsMakeMediaPacket(uint32_t frameCnt, unsigned char* dstMedia, void* pstStream)
{
    int buf_len = 0;
    unsigned int i;
    uint32_t frameType = 0;
    VENC_STREAM_S *pstH26XStream = (VENC_STREAM_S *)pstStream;
    VENC_PACK_S *ppack;
    unsigned char* h26x_buffer_media = dstMedia + H26X_FRAME_OFFSET + 2 * MAX_H26X_TAG_SIZE + H26X_HEADER_SIZE + MAX_H26X_FRAME_SIZE;
    for (i = 0; i < pstH26XStream->u32PackCount; ++i)
    {
        ppack = &pstH26XStream->pstPack[i];
        frameType = (uint32_t)ppack->DataType.enH265EType;
        if(buf_len + (ppack->u32Len - ppack->u32Offset) <= MAX_H26X_FRAME_SIZE){
            memcpy(h26x_buffer_media + buf_len, ppack->pu8Addr + ppack->u32Offset, ppack->u32Len - ppack->u32Offset);
            buf_len += (ppack->u32Len - ppack->u32Offset);
        }
        else{
            //overflow
            return -1;
        }
    }

    ds_mjpeg_app_header h26x_header = {0};
    if (buf_len > 2 * MAX_H26X_PACKET_SIZE) {
        h26x_header.app_mark = __bswap_16(0xFFE7);
        h26x_header.total_size = __bswap_16(0xFFFF);
        h26x_header.strm_type = (H26X_TYPE == PT_H265) ? 2 : 1;
        h26x_header.fps = 20;
        h26x_header.frm_type = (frameType == H264E_NALU_PSLICE) ? 2 : 1;
        h26x_header.checksum = (buf_len & 0xFF);
        //h26x_header.seq = __bswap_32(frameCnt);
        h26x_header.seq = frameCnt;
        h26x_header.data_size = __bswap_16(MAX_H26X_PACKET_SIZE + 2);
        memcpy(&dstMedia[H26X_FRAME_OFFSET], (uint8_t*)&h26x_header, H26X_HEADER_SIZE);
        memcpy(&dstMedia[H26X_FRAME_OFFSET + H26X_HEADER_SIZE], h26x_buffer_media, MAX_H26X_PACKET_SIZE);

        h26x_header.app_mark = __bswap_16(0xFFE8);
        h26x_header.total_size = __bswap_16(0xFFFF);
        h26x_header.strm_type = (H26X_TYPE == PT_H265) ? 2 : 1;
        h26x_header.fps = 20;
        h26x_header.frm_type = (frameType == H264E_NALU_PSLICE) ? 2 : 1;
        h26x_header.checksum = 0;
        //h26x_header.seq = __bswap_32(frameCnt);
        h26x_header.seq = frameCnt;
        h26x_header.data_size = __bswap_16(MAX_H26X_PACKET_SIZE + 2);
        memcpy(&dstMedia[H26X_FRAME_OFFSET + MAX_H26X_TAG_SIZE], (uint8_t*)&h26x_header, H26X_HEADER_SIZE);
        memcpy(&dstMedia[H26X_FRAME_OFFSET + MAX_H26X_TAG_SIZE + H26X_HEADER_SIZE], &h26x_buffer_media[MAX_H26X_PACKET_SIZE], MAX_H26X_PACKET_SIZE);

        h26x_header.app_mark = __bswap_16(0xFFE9);
        h26x_header.total_size = __bswap_16(buf_len - (2 * MAX_H26X_PACKET_SIZE) + 12);
        h26x_header.strm_type = (H26X_TYPE == PT_H265) ? 2 : 1;
        h26x_header.fps = 20;
        h26x_header.frm_type = (frameType == H264E_NALU_PSLICE) ? 2 : 1;
        h26x_header.checksum = 0;
        //h26x_header.seq = __bswap_32(frameCnt);
        h26x_header.seq = frameCnt;
        h26x_header.data_size = __bswap_16(buf_len - (2 * MAX_H26X_PACKET_SIZE) + 2);
        memcpy(&dstMedia[H26X_FRAME_OFFSET + 2 * MAX_H26X_TAG_SIZE], (uint8_t*)&h26x_header, H26X_HEADER_SIZE);
        memcpy(&dstMedia[H26X_FRAME_OFFSET + 2 * MAX_H26X_TAG_SIZE + H26X_HEADER_SIZE], &h26x_buffer_media[2 * MAX_H26X_PACKET_SIZE], buf_len - (2 * MAX_H26X_PACKET_SIZE));
        buf_len += 3 * H26X_HEADER_SIZE;
    } else if (buf_len > MAX_H26X_PACKET_SIZE) {
        h26x_header.app_mark = __bswap_16(0xFFE7);
        h26x_header.total_size = __bswap_16(0xFFFF);
        h26x_header.strm_type = (H26X_TYPE == PT_H265) ? 2 : 1;
        h26x_header.fps = 20;
        h26x_header.frm_type = (frameType == H264E_NALU_PSLICE) ? 2 : 1;
        h26x_header.checksum = (buf_len & 0xFF);
        //h26x_header.seq = __bswap_32(frameCnt);
        h26x_header.seq = frameCnt;
        h26x_header.data_size = __bswap_16(MAX_H26X_PACKET_SIZE + 2);
        memcpy(&dstMedia[H26X_FRAME_OFFSET], (uint8_t*)&h26x_header, H26X_HEADER_SIZE);
        memcpy(&dstMedia[H26X_FRAME_OFFSET + H26X_HEADER_SIZE], h26x_buffer_media, MAX_H26X_PACKET_SIZE);

        h26x_header.app_mark = __bswap_16(0xFFE8);
        h26x_header.total_size = __bswap_16(buf_len - MAX_H26X_PACKET_SIZE + 12);
        h26x_header.strm_type = (H26X_TYPE == PT_H265) ? 2 : 1;
        h26x_header.fps = 20;
        h26x_header.frm_type = (frameType == H264E_NALU_PSLICE) ? 2 : 1;
        h26x_header.checksum = 0;
        //h26x_header.seq = __bswap_32(frameCnt);
        h26x_header.seq = frameCnt;
        h26x_header.data_size = __bswap_16(buf_len - MAX_H26X_PACKET_SIZE + 2);
        memcpy(&dstMedia[H26X_FRAME_OFFSET + MAX_H26X_TAG_SIZE], (uint8_t*)&h26x_header, H26X_HEADER_SIZE);
        memcpy(&dstMedia[H26X_FRAME_OFFSET + MAX_H26X_TAG_SIZE + H26X_HEADER_SIZE], &h26x_buffer_media[MAX_H26X_PACKET_SIZE], buf_len - MAX_H26X_PACKET_SIZE);
        buf_len += 2 * H26X_HEADER_SIZE;
    } else if (buf_len > 0){
        h26x_header.app_mark = __bswap_16(0xFFE7);
        h26x_header.total_size = __bswap_16(buf_len + 12);
        h26x_header.strm_type = (H26X_TYPE == PT_H265) ? 2 : 1;
        h26x_header.fps = 20;
        h26x_header.frm_type = (frameType == H264E_NALU_PSLICE) ? 2 : 1;
        h26x_header.checksum = (buf_len & 0xFF);
        //h26x_header.seq = __bswap_32(frameCnt);
        h26x_header.seq = frameCnt;
        h26x_header.data_size = __bswap_16(buf_len + 2);
        memcpy(&dstMedia[H26X_FRAME_OFFSET], (uint8_t*)&h26x_header, H26X_HEADER_SIZE);
        memcpy(&dstMedia[H26X_FRAME_OFFSET + H26X_HEADER_SIZE], h26x_buffer_media, buf_len);
        buf_len += H26X_HEADER_SIZE;
    }
    return buf_len;
}

#endif // USE_USB_XN_PROTO