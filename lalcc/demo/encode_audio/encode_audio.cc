// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

#include "liblalcc/lalcc.hpp"
#include "chef_base/chef_stuff_op.hpp"

// aac (LC), 48000 Hz, stereo, fltp

int main() {
  // sample_fmts
  AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
  if (!codec) {
    LALCC_LOG_INFO("avcodec_find_decoder failed.");
    return 1;
  }

  AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
  if (!codecCtx) {
    LALCC_LOG_INFO("avcodec_alloc_context3 failed.");
    return 1;
  }

  // 64000 44100  -> 6, 21 10 04 60 8c 1c
  // 128000 44100 -> 6, 21 10 04 60 8c 1c
  // 128000 48000 -> 6, 21 10 04 60 8c 1c
  codecCtx->bit_rate = 128000; // 64000; // 128000;
  codecCtx->sample_rate = 48000; // 44100; // 48000;

  // 注意，aac即使设置了sample_fmt，下面avcodec_open2也会将sample_fmt重置为AV_SAMPLE_FMT_FLTP
  codecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;

  codecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
  codecCtx->channels = av_get_channel_layout_nb_channels(codecCtx->channel_layout);

  int ret = avcodec_open2(codecCtx, codec, NULL);
  if (ret < 0) {
    LALCC_LOG_INFO("avcodec_open2 failed. ret=%s", lalcc::HelperOp::StringifyAvError(ret).c_str());
    return 1;
  }

  AVPacket *pkt = av_packet_alloc();
  AVFrame *frame = av_frame_alloc();
  frame->nb_samples = codecCtx->frame_size;
  frame->format = codecCtx->sample_fmt;
  frame->channel_layout = codecCtx->channel_layout;

  ret = av_frame_get_buffer(frame, 0);
  if (ret < 0) {
    LALCC_LOG_INFO("av_frame_get_buffer failed. ret=%s", lalcc::HelperOp::StringifyAvError(ret).c_str());
    return 1;
  }

  for (int i = 0; i < 10; i++) {
    ret = av_frame_make_writable(frame);
    if (ret < 0) {
      LALCC_LOG_INFO("av_frame_make_writable failed. ret=%s", lalcc::HelperOp::StringifyAvError(ret).c_str());
      return 1;
    }

    for (int j = 0; j < codecCtx->frame_size; j++) {
      float *samples = (float *)frame->data[0];
      samples[j] = (float)0;
      samples = (float *)frame->data[1];
      samples[j] = (float)0;
    }

      ret = avcodec_send_frame(codecCtx, frame);
      if (ret < 0) {
        LALCC_LOG_INFO("avcodec_send_frame failed. ret=%s", lalcc::HelperOp::StringifyAvError(ret).c_str());
        return 1;
      }

      while (ret >= 0) {
        ret = avcodec_receive_packet(codecCtx, pkt);
        if (ret < 0) {
          if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            LALCC_LOG_INFO("avcodec_send_frame break. ret=%s", lalcc::HelperOp::StringifyAvError(ret).c_str());
            break;
          }
          LALCC_LOG_INFO("avcodec_send_frame failed. ret=%s", lalcc::HelperOp::StringifyAvError(ret).c_str());
          return 1;
        }

        LALCC_LOG_INFO("%d, %s", pkt->size, chef::stuff_op::bytes_to_hex(pkt->data, pkt->size).c_str());
      }
  }

  return 0;
}