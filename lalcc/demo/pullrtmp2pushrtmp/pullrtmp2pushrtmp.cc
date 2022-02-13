// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

#include <stdio.h>
#include "liblalcc/lalcc.hpp"

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

// 演示管理转推任务
// - 拉一路进行转推
// - 上层获取状态
// - 更换拉流，推流不停止
// - 更换推流，拉流不停止

int maxCount = 1024;

int count = 0;
lalcc::PullSessionPtr pullSession;
lalcc::PushSessionPtr pushSession;
lalcc::DecodePtr  decode;
char *pushRtmpUrl;

void onStreamInfo(AVFormatContext *fmtCtx) {
  LALCC_LOG_INFO("-----nb_streams=%d", fmtCtx->nb_streams);
  for (unsigned int i=0; i<fmtCtx->nb_streams;i++) {
    AVStream *stream = fmtCtx->streams[i];
    LALCC_LOG_INFO("stream=%s", lalcc::HelperOp::StringifyAvStream(stream).c_str());
    LALCC_LOG_INFO("codec param=%s", lalcc::HelperOp::StringifyAvCodecParameters(stream->codecpar).c_str());
    LALCC_LOG_INFO("%s", chef::stuff_op::bytes_to_hex(stream->codecpar->extradata, std::min(stream->codecpar->extradata_size, 128)).c_str());

    if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      decode = lalcc::NewDecode();
      decode->Open(stream);
    }
  }

//  pushSession = lalcc::NewPushSession();
//  int ret = pushSession->Push(pushRtmpUrl, fmtCtx);
//  if (ret != 0) {
//    LALCC_LOG_INFO("Push failed. ret=%d", ret);
//  }

}

void onAvPacket(lalcc::AvPacketTPtr pkt, AVStream *stream) {
//  LALCC_LOG_INFO("stream=%s", lalcc::HelperOp::StringifyAvStream(stream).c_str());
//  LALCC_LOG_INFO("pkt=%s", lalcc::HelperOp::StringifyAvPacket(pkt->Core()).c_str());
  //LALCC_LOG_INFO("%s", chef::stuff_op::bytes_to_hex(pkt->Core()->data, std::min(pkt->Core()->size, 16)).c_str());
  if (count++ > maxCount) {
    pullSession->Dispose();
  }

//  int ret = pushSession->Write(pkt, stream->codecpar->codec_type, stream->time_base);
//  if (ret != 0) {
//    LALCC_LOG_INFO("Write faileed. ret=%d", ret);
//  }

  if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
    decode->Push(pkt);
  }
}

int main(int argc, char **argv) {
  LALCC_LOG_INFO("%s", lalcc::HelperOp::StringifyAvError(-22).c_str());
  (void)argc;
  const char *pullRtmpUrl = argv[1];
  pushRtmpUrl = argv[2];

  LALCC_LOG_INFO("> main. %s, %s", pullRtmpUrl, pushRtmpUrl);

  pullSession = lalcc::NewPullSession();
  int ret = pullSession->Pull(pullRtmpUrl, onStreamInfo, onAvPacket, 0, 0);
  if (ret != 0) {
    LALCC_LOG_INFO("pull failed. ret=%s", lalcc::HelperOp::StringifyAvError(ret).c_str());
    return -1;
  }

  LALCC_LOG_INFO("bye. %d", chef::stuff_op::tick_msec());
  return 0;
}

