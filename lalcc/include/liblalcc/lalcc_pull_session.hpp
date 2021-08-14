// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _PULL_SESSION_HPP_
#define _PULL_SESSION_HPP_

#include "lalcc_forward_declaration.hpp"
#include "lalcc_av_packet_t.hpp"
#include "lalcc_helper_op.hpp"

namespace lalcc {

  PullSessionPtr NewPullSession();

  class PullSession : public chef::enable_shared_from_this<PullSession> {
    public:
      typedef chef::function<void(AVFormatContext *fmtCtx)> OnStreamInfo;
      typedef chef::function<void(AvPacketTPtr pkt, AVStream *stream)> OnAvPacket;

      // 注意，一个对象只允许调用该函数一次
      //
      // @param onStreamInfo
      // @param onAvPacket
      //
      // @return 0 成功, 其他值为失败
      //
      int Pull(const char *url, OnStreamInfo onStreamInfo, OnAvPacket onAvPacket);

      // 主动关闭拉流
      //
      void Dispose();

    private:
      int pull(const char *url, OnStreamInfo onStreamInfo, OnAvPacket onAvPacket);

      void dispose();

    private:
      chef::atomic<bool> disposeFlag_;

      AVFormatContext *fmtCtx_=NULL;
  };

} // namespace lalcc

// --------------------------------------------------------------------------------------------------------------------

namespace lalcc {

  inline int PullSession::Pull(const char *url, OnStreamInfo onStreamInfo, OnAvPacket onAvPacket) {
    return pull(url, onStreamInfo, onAvPacket);
  }

  inline void PullSession::Dispose() {
    disposeFlag_ = true;
  }

  inline int PullSession::pull(const char *url, OnStreamInfo onStreamInfo, OnAvPacket onAvPacket) {
    int ret = HelperOp::AvformatOpenInput(&fmtCtx_, url, NULL, NULL, 1000);
    if (ret < 0) { return ret; }
    if (disposeFlag_) { return 0; }

    // 接收一部分音视频数据，从而分析这条连接上有哪些类型的流
    //
    ret = avformat_find_stream_info(fmtCtx_, NULL);
    if (ret < 0) { return ret; }
    if (disposeFlag_) { return 0; }

    //av_dump_format(fmtCtx, NULL, url, NULL);

    onStreamInfo(fmtCtx_);
    if (disposeFlag_) { return 0; }

    for (; ; ) {
      AvPacketTPtr pkt = chef::make_shared<AvPacketT>();

      ret = av_read_frame(fmtCtx_, pkt->Core());
      if (ret < 0) { return ret; }
      if (disposeFlag_) { return 0; }

      onAvPacket(pkt, fmtCtx_->streams[pkt->Core()->stream_index]);
    }

    // never reach here
    return 0;
  }

  inline void PullSession::dispose() {
    avformat_close_input(&fmtCtx_);
  }

  PullSessionPtr NewPullSession() {
    return chef::make_shared<PullSession>();
  }

} // namespace lalcc

#endif
