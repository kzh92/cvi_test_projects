/*
 * Copyright (C) 2019-2022 Alibaba Group Holding Limited
 */

#include <board.h>

#if !(defined(CONFIG_BOARD_AMP_LOAD_FW) && CONFIG_BOARD_AMP_LOAD_FW)

#include <memory>

#include <cxvision/cxvision.h>
#include <ulog/ulog.h>
#include <drv/tick.h>

#include "t_head_audio.h"

#define TAG "pre"

#define DATA_OUT_CHAN    3
#if defined(CONFIG_VOICE_DBUS_SUPPORT) && CONFIG_VOICE_DBUS_SUPPORT
#define PREPROC_IGN_CNT     80  /*唤醒后先忽略一定量数据再发送 */
#else
#define PREPROC_IGN_CNT     50  /*唤醒后先忽略一定量数据再发送 */
#endif

#define RUN_TIME_CHECK(intv) \
  while(0) { \
    static uint64_t checkcount = 0; \
    checkcount ++; \
    if (checkcount % intv == 0) { \
      LOGD(TAG, ">>>>count %ld, tickus %ld<<<<", checkcount, csi_tick_get_us()); \
    } \
  }

/* Debug */
// extern int g_pcminput_ignore_ssp;

using DataInputMessageT = thead::voice::proto::DataInputMsg;
using SspOutMessageT    = thead::voice::proto::SspOutMsg;

using SessionMessageT   = posto::Message<thead::voice::proto::SessionMsg>;
using RecordMessageT    = posto::Message<thead::voice::proto::RecordMsg>;

namespace cpt {

class PreProcess : public cx::PluginBase {
public:
  PreProcess();
  bool Init(const std::map<std::string, std::string>& props) override;
  bool DeInit() override;
  bool Process(const std::vector<cx::BufferPtr>& data_vec) override;

private:
  bool start_kws_;
  bool start_record_;
  int delay_cnt;

  std::shared_ptr<posto::Participant> participant_;
  std::shared_ptr<posto::Reader<SessionMessageT>> session_reader_;
  std::shared_ptr<posto::Reader<RecordMessageT>>  record_reader_;

private:
  void voice_session_sub(void);
  void voice_record(void);
};

PreProcess::PreProcess()
{
  start_kws_ = false;
  start_record_ = false;
  delay_cnt = 0;
}

void PreProcess::voice_session_sub(void) {
  session_reader_ = participant_->CreateReader<SessionMessageT>("SessionMsg",
      [this] (const std::shared_ptr<SessionMessageT>& msg) {
        LOGD(TAG, "SessionMsg got, cmd_id: %d", msg->body().cmd_id());
        switch (msg->body().cmd_id()) {
            case thead::voice::proto::BEGIN:
            start_kws_ = true;
            delay_cnt = 0;
            break;

            case thead::voice::proto::END:
            case thead::voice::proto::TIMEOUT:
            start_kws_ = false;
            break;

            default:
            break;
        }
    });
}

void PreProcess::voice_record(void) {
  record_reader_ = participant_->CreateReader<RecordMessageT>("RecordMsg",
      [this] (const std::shared_ptr<RecordMessageT>& msg) {

        LOGD(TAG, "voice_record Message got, cmd_id: %d", msg->body().cmd());
        switch (msg->body().cmd()) {
            case thead::voice::proto::START:
              start_record_ = true;
              break;

            case thead::voice::proto::STOP:
              start_record_ = false;
              break;

            default:
              break;
        }

    });
}

bool PreProcess::Init(const std::map<std::string, std::string>& props) {
  participant_ = posto::Domain::CreateParticipant("cmd_consumer");
  start_record_ = false;

  voice_session_sub();

  voice_record();

  int16_t nsmode  = 0;
  int16_t aecmode = 4;
  int16_t vadmode = 1;

  int ret = T_Head_audio_init(nsmode, aecmode, vadmode);

  start_kws_ = false;

  return ret;
}

bool PreProcess::DeInit() {
  T_Head_audio_free();
  return true;
}

bool PreProcess::Process(const std::vector<cx::BufferPtr>& data_vec) {

  int16_t *none_interleaved_buf_;

  // get input data
  auto iMemory = data_vec.at(0)->GetMemory(0);
  int16_t *data_in = (int16_t *)iMemory->data();

  auto iMeta = data_vec.at(0)->GetMetadata<DataInputMessageT>("alsa_param");

  // output data
  int data_out_len = iMeta->frame() * (iMeta->format() / 8) * DATA_OUT_CHAN;
  auto oMemory = cx::MemoryHelper::Malloc(data_out_len); // * 2
  int16_t* data_out = (int16_t *)oMemory->data();

  none_interleaved_buf_ = (int16_t *)malloc(iMeta->frame() * (iMeta->format() / 8) * iMeta->chn_num());
  /* data interleaved check */
  for (int j = 0; j < iMeta->frame(); j++) {
      for (int i = 0; i < iMeta->chn_num(); i++) {
          none_interleaved_buf_[iMeta->frame() * i + j] = data_in[iMeta->chn_num() * j + i];
      }
  }
  // ssp and vad process
  //RUN_TIME_CHECK(500);
  int ret = T_Head_audio_process(none_interleaved_buf_, none_interleaved_buf_ + iMeta->frame() * (iMeta->chn_num() - 1), data_out);
  // int ret = 0;
  //RUN_TIME_CHECK(500);

  /* 若忽略信号处理，直接输出原始MIC数据 */
  // if (g_pcminput_ignore_ssp) {
  //   memcpy(data_out, none_interleaved_buf_, data_out_len);
  // }

  free(none_interleaved_buf_);

  // 信号处理输出三路，顺序为：BSS、BSS、BF，参考音直接覆盖第三路的BF
  // memcpy(&data_out[iMeta->frame() * (DATA_OUT_CHAN - 1)], data_in + iMeta->frame * (iMeta->chn_num - 1), iMeta->frame() * sizeof(int16_t));

  auto oMeta = std::make_shared<SspOutMessageT>();
  oMeta->set_chn_num(DATA_OUT_CHAN);
  oMeta->set_vad_res(ret);
  oMeta->set_sample_rate(iMeta->sample_rate());
  oMeta->set_frame(iMeta->frame());

  auto output = std::make_shared<cx::Buffer>();
  output->AddMemory(oMemory);
  output->SetMetadata("ssp_param", oMeta);

  Send(0, output);     // inference

  /* 唤醒延时发送语音数据，屏蔽唤醒词*/
  if (start_kws_ == true) {
    if (delay_cnt++ <= PREPROC_IGN_CNT) {
      return true;
    }

    Send(1, output);     // post_process
    Send(2, output);     // vad
  }

  /* 5路数据录音，必须为交织格式 */
  if (start_record_ == true) {
    //TODO::
    Send(3, output);  //record_process#1
  }

  return true;
}

CX_REGISTER_PLUGIN(PreProcess);

}  // namespace cpt

#endif