// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _LALCC_RTMP_PUSH_SESSION_HPP_
#define _LALCC_RTMP_PUSH_SESSION_HPP_

#include "lalcc_forward_declaration.hpp"
#include "lalcc_av_packet_t.hpp"

namespace lalcc {

  RtmpPushSessionPtr NewRtmpPushSession();

  class RtmpPushSession {
    public:
      // @param iFmtCtx: 通过输入（比如PullSession）的AVFormatContext来初始化推流信息
      //
      int Push(const char *url, AVFormatContext *iFmtCtx);

      int Write(AvPacketTPtr pkt, AVMediaType type, AVRational timeBase);

    private:
      AVFormatContext *fmtCtx_=NULL;
  };

} // namespace lalcc

// --------------------------------------------------------------------------------------------------------------------

namespace lalcc {

  inline int RtmpPushSession::Push(const char *url, AVFormatContext *iFmtCtx) {
    int ret = avformat_alloc_output_context2(&fmtCtx_, NULL, "flv", url);
    if (ret < 0) {
      return ret;
    }

    for (unsigned int i = 0; i < iFmtCtx->nb_streams; i++) {
      AVStream *ostream = avformat_new_stream(fmtCtx_, NULL);
      if (ostream == NULL) {
        return -1;
      }

      ret = avcodec_parameters_copy(ostream->codecpar, iFmtCtx->streams[i]->codecpar);
      if (ret < 0) {
        return ret;
      }
      ostream->codecpar->codec_tag = 0;
    }

    if (!(fmtCtx_->oformat->flags & AVFMT_NOFILE)) {
      ret = avio_open(&fmtCtx_->pb, url, AVIO_FLAG_WRITE);
      if (ret < 0) {
        return ret;
      }
    }

    ret = avformat_write_header(fmtCtx_, NULL);
    if (ret < 0) {
      return ret;
    }

    return 0;
  }

  inline int RtmpPushSession::Write(AvPacketTPtr pkt, AVMediaType type, AVRational timeBase) {
    LALCC_LOG_INFO("> RtmpPushSession::Write");
    // Clone一下，从而保证我们不修改`pkt`的字段
    AvPacketTPtr wpkt = pkt->Clone();
    // TODO(chef): 如果timeBase和本地stream不一致，还需要调整pts, dts, duration
    wpkt->Core()->pos = -1;
    LALCC_LOG_INFO("wpkt=%s", lalcc::HelperOp::StringifyAvPacket(wpkt->Core()).c_str());

    //int ret = av_interleaved_write_frame(fmtCtx_, wpkt->Core());
    int ret = av_write_frame(fmtCtx_, wpkt->Core());
    LALCC_LOG_INFO("< writeframe. ret=%s", HelperOp::StringifyAvError(ret).c_str());
    if (ret < 0) {
      return ret;
    }

    return 0;
  }

  RtmpPushSessionPtr NewRtmpPushSession() {
    return chef::make_shared<RtmpPushSession>();
  }

}

#endif
