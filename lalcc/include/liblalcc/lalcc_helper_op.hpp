// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _LALCC_HELPER_HPP_
#define _LALCC_HELPER_HPP_

// lalcc_helper_op.hpp
//
// 静态的独立的帮助函数
//

#include "lalcc_forward_declaration.hpp"

namespace lalcc {

  class HelperOp {
    public:

      static std::string StringifyAvError(int code);

      static std::string StringifyAvStream(AVStream *stream);

      static std::string StringifyAvCodecParameters(AVCodecParameters *param);

      static std::string StringifyAvPacket(AVPacket *pkt);

      //static int ParseSpsPpsFromSeqHeaderWithoutMalloc(uint8_t *in, int inLen, uint8_t **sps, int *spsLen, uint8_t **pps, int *ppsLen);

      //static int ParseVpsSpsPpsFromSeqHeaderWithoutMalloc(uint8_t *in, int inLen, uint8_t **vps, int *vpsLen, uint8_t **sps, int *spsLen, uint8_t **pps, int *ppsLen);

      //static int parseSpsPpsFromSeqHeader(uint8_t *in, int inLen, uint8_t **sps, int *spsLen, uint8_t **pps, int *ppsLen);

      //static int parseVpsSpsPpsFromSeqHeader(uint8_t *in, int inLen, uint8_t **vps, int *vpsLen, uint8_t **sps, int *spsLen, uint8_t **pps, int *ppsLen);
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

  inline std::string HelperOp::StringifyAvError(int code) {
    std::ostringstream oss;
    char buf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_make_error_string(buf, AV_ERROR_MAX_STRING_SIZE, code);
    oss << "(" << code << ":" << buf << ")";
    return oss.str();
  }

  inline std::string HelperOp::StringifyAvStream(AVStream *stream) {
    char buf[1024] = {0};
    snprintf(buf, 1023, "%p, index=%d, id=%d, time_base=(%d/%d), start_time=%lld, duration=%lld",
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
        "%p, stream_index=%d, pts=%lld, dts=%lld, duration=%lld, data=%p, size=%d, flags=%d, pos=%lld, buf=%p, side_data=%p, side_data_elems=%d",
        pkt, pkt->stream_index, pkt->pts, pkt->dts, pkt->duration, pkt->data, pkt->size,
        pkt->flags, pkt->pos, pkt->buf, pkt->side_data, pkt->side_data_elems);
    return std::string(buf);
  }

  //inline int HelperOp::ParseVpsSpsPpsFromSeqHeaderWithoutMalloc(uint8_t *in, int inLen, uint8_t **vps, int *vpsLen, uint8_t **sps, int *spsLen, uint8_t **pps, int *ppsLen) {

  //}

  //inline int HelperOp::parseVpsSpsPpsFromSeqHeader(uint8_t *in, int inLen, uint8_t **vps, int *vpsLen, uint8_t **sps, int *spsLen, uint8_t **pps, int *ppsLen) {
  //  if (inLen < 33 - 5) {
  //    return -1;
  //  }

  //  int index = 27 - 5;
  //  int numOfArrays = in[index];
  //  if (numOfArrays != 3 && numOfArrays != 4) {
  //    return -1;
  //  }
  //  index++;

  //}

} // namespace lalcc

#endif
