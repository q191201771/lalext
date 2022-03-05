// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _LALCC_DECODE_HPP_
#define _LALCC_DECODE_HPP_

// lalcc_decode.hpp
//
// 没有完成
//

#include "lalcc_forward_declaration.hpp"
#include "lalcc_av_packet_t.hpp"

namespace lalcc {

  DecodePtr NewDecode();

  class Decode {
    public:
      int Open(AVStream *stream);
      int Push(AvPacketTPtr pkt);

      int Open();
      int Push(uint8_t *data, int dataSize);

    private:
      AVCodecContext *codecCtx_ = nullptr;
      AVCodecParserContext *parser_ = nullptr;
      SwrContext *swrCtx_ = nullptr;
      FILE *fp_ = nullptr;
  };

}

// --------------------------------------------------------------------------------------------------------------------

namespace lalcc {

  inline int Decode::Open(AVStream *stream) {
    int ret;
    auto tmp = chef::stuff_op::bytes_to_hex(stream->codecpar->extradata, stream->codecpar->extradata_size);
	LALCC_LOG_INFO("%s", tmp.c_str());

    AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
      return -1;
    }

    codecCtx_ = avcodec_alloc_context3(codec);
    if (!codecCtx_) {
      return -1;
    }

    ret = avcodec_parameters_to_context(codecCtx_, stream->codecpar);
    if (ret < 0) {
      return ret;
    }

    ret = avcodec_open2(codecCtx_, codec, nullptr);
    if (ret < 0) {
      return ret;
    }

//    swrCtx_ = swr_alloc();
//    // 注意，有些字段是必须填写的，否则会init失败，具体见内部实现
//    av_opt_set_int(swrCtx_, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
//    av_opt_set_int(swrCtx_, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
//    av_opt_set_int(swrCtx_, "in_sample_rate", 48000, 0);
//    av_opt_set_int(swrCtx_, "out_sample_rate", 48000, 0);
//    av_opt_set_sample_fmt(swrCtx_, "in_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
//    av_opt_set_sample_fmt(swrCtx_, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
//    // (-22:Invalid argument)
//    ret = swr_init(swrCtx_);
//    if (ret < 0) {
//      return -1;
//    }

	fp_ = fopen("out.pcm", "w");
	if (fp_ == nullptr) {
	  return -1;
	}

    return 0;
  }

  inline int Decode::Open() {
    AVCodecID codecId = AV_CODEC_ID_AAC;
    int ret;

    AVCodec *codec = avcodec_find_decoder(codecId);
    if (!codec) {
      return -1;
    }

    codecCtx_ = avcodec_alloc_context3(codec);
    if (!codecCtx_) {
      return -1;
    }


    AVCodecParameters *param = avcodec_parameters_alloc();
    param->codec_type = AVMEDIA_TYPE_AUDIO;
	param->channels = 2;
	param->sample_rate = 48000;
//	param->codec_id = AV_CODEC_ID_AAC;
//	param->extradata = new uint8_t[2];
//	param->extradata[0] = 0x11;
//	param->extradata[1] = 0x90;
//	param->extradata_size = 2;
//
//	param->codec_tag = 0;
//	param->format = AV_SAMPLE_FMT_FLTP;
//	param->bits_per_coded_sample = 16;
//	param->channel_layout = 3;
//	param->frame_size = 1024;

	ret = avcodec_parameters_to_context(codecCtx_, param);
	if (ret < 0) {
	  return ret;
	}

	ret = avcodec_open2(codecCtx_, codec, nullptr);
	if (ret < 0) {
	  return ret;
	}

	parser_ = av_parser_init(codecId);

    fp_ = fopen("out.pcm", "w");
    if (fp_ == nullptr) {
      return -1;
    }

    return 0;
  }

  inline int Decode::Push(uint8_t *data, int dataSize) {
    int ret;

    auto pkt = lalcc::NewAvPacketT();
    pkt->Core()->data = data;
    pkt->Core()->size = dataSize;
//    ret = av_parser_parse2(parser_, codecCtx_, &(pkt->Core()->data), &(pkt->Core()->size), data, dataSize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
//    if (ret < 0) {
//      return ret;
//    }
	if (pkt->Core()->size > 0) {
	  return Push(pkt);
	}
	return 0;
  }

  inline int Decode::Push(AvPacketTPtr pkt) {
    printf("> Push. inSize=%d %s\n", pkt->Core()->size, chef::stuff_op::bytes_to_hex(pkt->Core()->data, 16).c_str());

    AVFrame *frame = av_frame_alloc();
    int ret = avcodec_send_packet(codecCtx_, pkt->Core());
    if (ret < 0) {
      return ret;
    }
    while ( ret >= 0) {
      ret = avcodec_receive_frame(codecCtx_, frame);
      if (ret < 0) {
        if (ret == AVERROR(EAGAIN)) {
          return 0;
        }
        return ret;
      }

//      uint8_t *out = new uint8_t[1024 * 1024];
//      ret = swr_convert(swrCtx_, &out, frame->nb_samples, (const uint8_t **)frame->data, frame->nb_samples);
//	  if (ret < 0) {
//		return ret;
//	  }

	  int nbSamples = frame->nb_samples;
	  int channels = frame->channels;
      int dataSize = av_get_bytes_per_sample(codecCtx_->sample_fmt);

      LALCC_LOG_INFO("nbSamples=%d, dataSize=%d", nbSamples, dataSize);

      // 存储方式1 只存一个通道
//      for (int i = 0; i < frame->nb_samples; i++) {
//        fwrite(frame->data[0]+dataSize * i, dataSize, 1, fp_);
//      }

      // 存储方式2 存两个通道。planar转换成packed存储
      for (int i = 0; i < nbSamples; i++) {
        for (int j = 0; j < channels; j++) {
          fwrite(frame->data[j]+dataSize * i, dataSize, 1, fp_);
          fflush(fp_);
        }
      }
    }
  }

  inline DecodePtr NewDecode() {
    return chef::make_shared<Decode>();
  }

}

#endif