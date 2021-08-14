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
      // 封装avformat_open_input，增加一个参数，提供超时功能
      // 具体机制见内容实现的注释说明
      //
      static int AvformatOpenInput(AVFormatContext **ps, const char *url, ff_const59 AVInputFormat *fmt, AVDictionary **options, uint64_t timeoutMsec);

      static std::string StringifyAvError(int code);

      static std::string StringifyAvStream(AVStream *stream);

      static std::string StringifyAvCodecParameters(AVCodecParameters *param);

      static std::string StringifyAvPacket(AVPacket *pkt);
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

  inline int HelperOp::AvformatOpenInput(AVFormatContext **ps, const char *url, ff_const59 AVInputFormat *fmt, AVDictionary **options, uint64_t timeoutMsec) {
    if (*ps == NULL) {
      *ps = avformat_alloc_context();
    }
    OpenTimeoutHooker oth;
    oth.CallMeBeforeOpen(*ps, timeoutMsec);
    return avformat_open_input(ps, url, fmt, options);
  }

  inline std::string HelperOp::StringifyAvError(int code) {
    std::ostringstream oss;
    char buf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_make_error_string(buf, AV_ERROR_MAX_STRING_SIZE, code);
    oss << "(" << code << ":" << buf << ")";
    return oss.str();
  }

  inline std::string HelperOp::StringifyAvStream(AVStream *stream) {
    char buf[1024] = {0};
    snprintf(buf, 1023, "%p, index=%d, id=%d, time_base=(%d/%d), start_time=%d, duration=%d",
        stream, stream->index, stream->id, stream->time_base.num, stream->time_base.den, stream->start_time, stream->duration);
    return std::string(buf);
  }

  inline std::string HelperOp::StringifyAvCodecParameters(AVCodecParameters *param) {
    char buf[1024] = {0};
    snprintf(buf, 1024, "%p codec_type=(%d, %s), codec_id=(%d, %s), codec_tag=%d, extradata_size=%d, format=%d",
        param, param->codec_type, av_get_media_type_string(param->codec_type), param->codec_id, avcodec_get_name(param->codec_id), param->codec_tag, param->extradata_size, param->format);
    return std::string(buf);
  }

  inline std::string HelperOp::StringifyAvPacket(AVPacket *pkt) {
    char buf[1024] = {0};
    snprintf(buf, 1023,
        "%p, stream_index=%d, pts=%lld, dts=%lld, duration=%lld, data=%p, size=%d, flags=%d, pos=%d, buf=%p, side_data=%p, side_data_elems=%d",
        pkt, pkt->stream_index, pkt->pts, pkt->dts, pkt->duration, pkt->data, pkt->size,
        pkt->flags, pkt->pos, pkt->buf, pkt->side_data, pkt->side_data_elems);
    return std::string(buf);
  }

} // namespace lalcc

#endif
