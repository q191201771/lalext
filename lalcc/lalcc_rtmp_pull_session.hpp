// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _RTMP_PULL_SESSION_HPP_
#define _RTMP_PULL_SESSION_HPP_

#include "lalcc_forward_declaration.hpp"
#include "lalcc_helper_op.hpp"

namespace lalcc {

  class RtmpPullSession : public chef::enable_shared_from_this<RtmpPullSession> {
    public:
      typedef chef::function<void(AVFormatContext *fmtCtx)> OnStreamInfo;
      typedef chef::function<void(AVPacket *pkt)> OnAvPacket;

      // 阻塞直到拉流通道建立成功或失败
      //
      // 注意，这里不仅会完成信令的交互，还会接收一部分音视频数据，从而分析这条连接上有哪些类型的流
      //
      // 注意，一个对象只允许调用该函数一次
      //
      // @param onStreamInfo
      // @param onAvPacket
      //
      // @return 0 成功, 其他值为失败
      //
      int Pull(const char *url, OnStreamInfo onStreamInfo, OnAvPacket onAvPacket);

      // 阻塞直到拉流结束
      //
      // 该函数可用于获取拉流结束的消息
      //
      // 注意，请在`Pull`函数成功的情况下使用
      //
      // @return 0 表示外层主动关闭；非0 表示内部关闭，比如网络断开或对端关闭
      //
      int Wait();

      // 主动关闭拉流
      //
      void Dispose();

    private:
      int pull(const char *url, OnStreamInfo onStreamInfo, OnAvPacket onAvPacket);

      void dispose();

      void notifyFirstStage(int n);
      void notifySecondStage(int n);
      int waitFirstStage();
      int waitSecondStage();

    private:
      chef::task_thread taskThread_;

      chef::atomic<bool> disposeFlag_;

      chef::wait_event_counter wecFirstStage_;
      chef::wait_event_counter wecSecondStage_;
      chef::atomic<int> firstStageResult_;
      chef::atomic<int> secondStageResult_;

      AVFormatContext *fmtCtx_=NULL;
  };

  RtmpPullSessionPtr NewRtmpPullSession() {
    return chef::make_shared<RtmpPullSession>();
  }

} // namespace lalcc

// --------------------------------------------------------------------------------------------------------------------

namespace lalcc {

  inline int RtmpPullSession::Pull(const char *url, OnStreamInfo onStreamInfo, OnAvPacket onAvPacket) {
    taskThread_.start();
    taskThread_.add(chef::bind(&RtmpPullSession::pull, shared_from_this(), url, onStreamInfo, onAvPacket));
    return waitFirstStage();
  }

  inline int RtmpPullSession::Wait() {
    return waitSecondStage();
  }

  inline void RtmpPullSession::Dispose() {
    disposeFlag_ = true;
  }

#define SNIPPET_RTMP_PULL_SESSION_CHECK_FLAG_AT_FIREST_STAGE() \
  { \
    if (disposeFlag_) { \
      dispose(); \
      notifyFirstStage(0); \
      return 0; \
    } \
  }

#define SNIPPET_RTMP_PULL_SESSION_CHECK_FLAG_AT_SECOND_STAGE() \
  { \
    if (disposeFlag_) { \
      dispose(); \
      notifySecondStage(0); \
      return 0; \
    } \
  }

#define SNIPPET_RTMP_PULL_SESSION_CHECK_RET_AT_FIREST_STAGE(ret) \
  { \
    if (ret != 0) { \
      dispose(); \
      notifyFirstStage(ret); \
      return ret; \
    } \
  }

#define SNIPPET_RTMP_PULL_SESSION_CHECK_RET_AT_SECOND_STAGE(ret) \
  { \
    if (ret != 0) { \
      dispose(); \
      notifySecondStage(ret); \
      return ret; \
    } \
  }

  inline int RtmpPullSession::pull(const char *url, OnStreamInfo onStreamInfo, OnAvPacket onAvPacket) {
    int ret = HelperOp::AvformatOpenInput(&fmtCtx_, url, NULL, NULL, 1000);
    SNIPPET_RTMP_PULL_SESSION_CHECK_RET_AT_FIREST_STAGE(ret);
    SNIPPET_RTMP_PULL_SESSION_CHECK_FLAG_AT_FIREST_STAGE();

    // 接收一部分音视频数据，从而分析这条连接上有哪些类型的流
    //
    ret = avformat_find_stream_info(fmtCtx_, NULL);
    SNIPPET_RTMP_PULL_SESSION_CHECK_RET_AT_FIREST_STAGE(ret);
    SNIPPET_RTMP_PULL_SESSION_CHECK_FLAG_AT_FIREST_STAGE();

    //av_dump_format(fmtCtx, NULL, rtmpUrl, NULL);

    // 第一阶段正常结束
    //
    notifyFirstStage(0);

    onStreamInfo(fmtCtx_);

    for (; ; ) {
      AVPacket *pkt = av_packet_alloc();

      ret = av_read_frame(fmtCtx_, pkt);
      SNIPPET_RTMP_PULL_SESSION_CHECK_RET_AT_SECOND_STAGE(ret);
      SNIPPET_RTMP_PULL_SESSION_CHECK_FLAG_AT_SECOND_STAGE();
      onAvPacket(pkt);

      av_packet_unref(pkt);
    }

    // never reach here
    return 0;
  }

  inline void RtmpPullSession::dispose() {
    avformat_close_input(&fmtCtx_);
  }

  inline void RtmpPullSession::notifyFirstStage(int n) {
    firstStageResult_ = n;
    wecFirstStage_.notify();
  }

  inline void RtmpPullSession::notifySecondStage(int n) {
    secondStageResult_ = n;
    wecSecondStage_.notify();
  }

  inline int RtmpPullSession::waitFirstStage() {
    wecFirstStage_.wait();
    return firstStageResult_;
  }

  inline int RtmpPullSession::waitSecondStage() {
    wecSecondStage_.wait();
    return secondStageResult_;
  }

} // namespace lalcc

#endif
