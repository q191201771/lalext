// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _LALCC_HELPER_HPP_
#define _LALCC_HELPER_HPP_

#include "lalcc_forward_declaration.hpp"

namespace lalcc {

  class HelperOp {
    public:
      static std::string AvErr2Str(int code);

      // 封装avformat_open_input，增加一个参数，提供超时功能
      // 具体机制见内容实现的注释说明
      //
      static int AvformatOpenInput(AVFormatContext **ps, const char *url, ff_const59 AVInputFormat *fmt, AVDictionary **options, uint64_t timeoutMsec);
  };

} // namespace lalcc

// --------------------------------------------------------------------------------------------------------------------

namespace lalcc {

class OpenTimeoutHooker {
  public:
    // 调用avformat_open_input前调用该函数，设置超时
    // 注意，只是设置avformat_open_input这一步的超时时间
    //
    // @param timeoutMsec 超时时间，单位毫秒，如果为0，则没有超时
    //
    void CallMeBeforeOpen(AVFormatContext *fmtCtx, uint64_t timeoutMsec) {
      timeoutMsec_ = timeoutMsec;
      openTickMsec_ = chef::stuff_op::tick_msec();

      // 回调只和avformat_open_input阶段相关，avformat_open_input返回前，回调被持续调用，avformat_open_input返回后，回调不再被调用
      // 所以这个方式只能用来做avformat_open_input的超时逻辑
      //
      // case 1
      // 拉取不存在的rtmp流，avformat_open_input没有返回，回调依然一直被调用
      //
      // case 2
      // 拉取存在的rtmp流，avformat_open_input返回成功后，回调不再被调用
      //
      // TODO(chef): 跟ffmpeg的代码
      //
      fmtCtx->interrupt_callback.callback = OpenTimeoutHooker::interrupt_cb;
      fmtCtx->interrupt_callback.opaque = (void *)this;
    }

  private:
    static int interrupt_cb(void *opaque) {
      OpenTimeoutHooker *obj = (OpenTimeoutHooker *)opaque;
      if (obj->timeoutMsec_ != 0 && (chef::stuff_op::tick_msec() - obj->openTickMsec_ > obj->timeoutMsec_) ) {
        // 超时了，返回1关闭
        //
        return 1;
      }
      //printf("interrupt_cb %d\n", chef::stuff_op::tick_msec());
      return 0;
    }

  private:
    uint64_t openTickMsec_ = 0;
    uint64_t timeoutMsec_ = 0;
};

} // namespace lalcc

// --------------------------------------------------------------------------------------------------------------------

namespace lalcc {

  inline std::string HelperOp::AvErr2Str(int code) {
    std::ostringstream oss;
    char buf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_make_error_string(buf, AV_ERROR_MAX_STRING_SIZE, code);
    oss << "(" << code << ":" << buf << ")";
    return oss.str();
  }

  inline int HelperOp::AvformatOpenInput(AVFormatContext **ps, const char *url, ff_const59 AVInputFormat *fmt, AVDictionary **options, uint64_t timeoutMsec) {
    if (*ps == NULL) {
      *ps = avformat_alloc_context();
    }
    OpenTimeoutHooker oth;
    oth.CallMeBeforeOpen(*ps, timeoutMsec);
    return avformat_open_input(ps, url, fmt, options);
  }

} // namespace lalcc

#endif
