// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

#include <stdio.h>
#include "liblalcc/lalcc.hpp"

int maxCount = 10;

int count = 0;
lalcc::PullSessionPtr pullSession;

void onStreamInfo(AVFormatContext *fmtCtx) {
  LALCC_LOG_INFO("-----nb_streams=%d", fmtCtx->nb_streams);
  for (unsigned int i=0; i<fmtCtx->nb_streams;i++) {
    AVStream *stream = fmtCtx->streams[i];
    LALCC_LOG_INFO("stream=%s", lalcc::HelperOp::StringifyAvStream(stream).c_str());
    LALCC_LOG_INFO("codec param=%s", lalcc::HelperOp::StringifyAvCodecParameters(stream->codecpar).c_str());
    LALCC_LOG_INFO("%s", chef::stuff_op::bytes_to_hex(stream->codecpar->extradata, std::min(stream->codecpar->extradata_size, 128)).c_str());

  }
}

void onAvPacket(lalcc::AvPacketTPtr pkt, AVStream *stream) {
  LALCC_LOG_INFO("stream=%s", lalcc::HelperOp::StringifyAvStream(stream).c_str());
  LALCC_LOG_INFO("pkt=%s", lalcc::HelperOp::StringifyAvPacket(pkt->Core()).c_str());
  //LALCC_LOG_INFO("%s", chef::stuff_op::bytes_to_hex(pkt->Core()->data, std::min(pkt->Core()->size, 16)).c_str());
  if (count++ > maxCount) {
    pullSession->Dispose();
  }
}

int main(int argc, char **argv) {
  const char *pullRtmpUrl = argv[1];

  LALCC_LOG_INFO("> main. %s", pullRtmpUrl);

  pullSession = lalcc::NewPullSession();
  int ret = pullSession->Pull(pullRtmpUrl, onStreamInfo, onAvPacket);
  if (ret != 0) {
    LALCC_LOG_INFO("pull failed. ret=%s", lalcc::HelperOp::StringifyAvError(ret).c_str());
    return -1;
  }

  LALCC_LOG_INFO("bye. %d", chef::stuff_op::tick_msec());
  return 0;
}
