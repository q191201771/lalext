// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#include <stdio.h>
#include "lalcc.hpp"

// TODO(chef):
// - RtmpPullSession的配置项
// - 读取数据的超时，目前业务方可以临时自己写上层逻辑判断多久没有收到数据关闭session
// - 工程方面的接口
// - 音频和视频都存成ES格式，加深对数据格式的理解
// - 转推
//
// - 拉流
// - 解码
// - 编码
// - 合流

int maxCount = 128 * 1024;

char *pullRtmpUrl = "rtmp://127.0.0.1/live/test110";
char *pushRtmpUrl = "rtmp://127.0.0.1/live/test220";

int count = 0;
lalcc::RtmpPullSessionPtr pullSession;
lalcc::RtmpPushSessionPtr pushSession;

void onStreamInfo(AVFormatContext *fmtCtx) {
  LALCC_LOG_INFO("-----nb_streams=%d", fmtCtx->nb_streams);
  for (unsigned int i=0; i<fmtCtx->nb_streams;i++) {
    AVStream *stream = fmtCtx->streams[i];
    AVCodecParameters *codecpar = stream->codecpar;
    enum AVCodecID codecId = codecpar->codec_id;
    LALCC_LOG_INFO("stream=%s", lalcc::HelperOp::StringifyAvStream(stream).c_str());
    LALCC_LOG_INFO("codec_id=%s, %s", avcodec_get_name(codecId), av_get_media_type_string(avcodec_get_type(codecId)));
    LALCC_LOG_INFO("%s", chef::stuff_op::bytes_to_hex(codecpar->extradata, std::min(codecpar->extradata_size, 16)).c_str());
  }

  pushSession = lalcc::NewRtmpPushSession();
  int ret = pushSession->Push(pushRtmpUrl, fmtCtx);
  if (ret < 0) {
    LALCC_LOG_INFO("push failed. ret=%d", ret);
    //exit(-1);
  }
}

void onAvPacket(lalcc::AvPacketTPtr pkt, AVStream *stream) {
  LALCC_LOG_INFO("stream=%s", lalcc::HelperOp::StringifyAvStream(stream).c_str());
  LALCC_LOG_INFO("pkt=%s", lalcc::HelperOp::StringifyAvPacket(pkt->Core()).c_str());
  LALCC_LOG_INFO("%s", chef::stuff_op::bytes_to_hex(pkt->Core()->data, std::min(pkt->Core()->size, 16)).c_str());
  if (count++ > maxCount) {
    pullSession->Dispose();
  }

  int ret = pushSession->Write(pkt, stream->codecpar->codec_type, stream->time_base);
  if (ret < 0) {
    LALCC_LOG_INFO("push session write failed. err=%s", lalcc::HelperOp::AvErr2Str(ret).c_str());
  }
}

int main(int argc, char **argv) {
  LALCC_LOG_INFO("> main. %d", chef::stuff_op::tick_msec());

  pullSession = lalcc::NewRtmpPullSession();
  int ret = pullSession->Pull(pullRtmpUrl, onStreamInfo, onAvPacket);
  if (ret != 0) {
    LALCC_LOG_INFO("pull failed. ret=%s", lalcc::HelperOp::AvErr2Str(ret).c_str());
    return -1;
  }

  LALCC_LOG_INFO("> Wait.");
  ret = pullSession->Wait();
  LALCC_LOG_INFO("< Wait. ret=%s", lalcc::HelperOp::AvErr2Str(ret).c_str());

  LALCC_LOG_INFO("bye. %d", chef::stuff_op::tick_msec());
  return 0;
}
