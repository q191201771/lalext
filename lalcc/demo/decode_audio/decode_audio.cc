// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

#include <stdio.h>
#include "liblalcc/lalcc.hpp"

lalcc::DecodePtr decode;

void OnStreamInfo(AVFormatContext *fmtCtx) {

}

void OnAvPacket(lalcc::AvPacketTPtr pkt, AVStream *stream) {
  int ret;

  if (stream->codecpar->codec_id == AV_CODEC_ID_AAC) {
    if (!decode.get()) {
	  decode = lalcc::NewDecode();
//	  ret = decode->Open(stream);
	  ret = decode->Open();
	  if (ret < 0) {
		LALCC_LOG_INFO("ret=%d(%s)", ret, lalcc::HelperOp::StringifyAvError(ret).c_str());
		exit(-1);
	  }
    }

//	ret = decode->Push(pkt);
	ret = decode->Push(pkt->Core()->data, pkt->Core()->size);
	if (ret < 0) {
	  LALCC_LOG_INFO("ret=%d(%s)", ret, lalcc::HelperOp::StringifyAvError(ret).c_str());
//	  exit(-1);
    }
  }

//  LALCC_LOG_INFO("----------\n%s\n%s\n%s\n%s",
//				 lalcc::HelperOp::StringifyAvStream(stream).c_str(),
//				 lalcc::HelperOp::StringifyAvCodecParameters(stream->codecpar).c_str(),
//				 lalcc::HelperOp::StringifyAvPacket(pkt->Core()).c_str(),
//				 chef::stuff_op::bytes_to_hex(pkt->Core()->data, 32).c_str());
}

int main(int argc, char **argv) {
  auto url = argv[1];

  auto pullSession = lalcc::NewPullSession();
  auto ret = pullSession->Pull(url, OnStreamInfo, OnAvPacket, 0, 0);
  LALCC_LOG_INFO("ret=%d(%s)", ret, lalcc::HelperOp::StringifyAvError(ret).c_str());

  return 0;
}