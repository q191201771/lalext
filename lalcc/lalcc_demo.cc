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
//
// - 转推
// - 拉流
// - 解码
// - 编码
// - 合流

int count = 0;
lalcc::RtmpPullSessionPtr session;

void onStreamInfo(AVFormatContext *fmtCtx) {
  printf("-----nb_streams=%d\n", fmtCtx->nb_streams);
  for (unsigned int i=0; i<fmtCtx->nb_streams;i++) {
    AVStream *stream = fmtCtx->streams[i];
    AVCodecParameters *codecpar = stream->codecpar;
    enum AVCodecID codecId = codecpar->codec_id;
    printf("codec_id=%s, %s\n", avcodec_get_name(codecId), av_get_media_type_string(avcodec_get_type(codecId)));
    printf("%s", chef::stuff_op::bytes_to_hex(codecpar->extradata, std::min(codecpar->extradata_size, 16)).c_str());
  }
}

void onAvPacket(AVPacket *pkt) {
  printf("-----%d, %7d, %lld\n", pkt->stream_index, pkt->size, pkt->dts);
  printf("%s", chef::stuff_op::bytes_to_hex(pkt->data, std::min(pkt->size, 16)).c_str());
  if (count++ > 128) {
    session->Dispose();
  }
}

int main(int argc, char **argv) {
  printf("> main. %d\n", chef::stuff_op::tick_msec());

  char *rtmpUrl = "rtmp://127.0.0.1/live/test110";
  if (argc >= 2) {
    rtmpUrl = argv[1];
  }

  session = lalcc::NewRtmpPullSession();
  int ret = session->Pull(rtmpUrl, onStreamInfo, onAvPacket);
  if (ret != 0) {
    printf("pull failed. ret=%s\n", lalcc::HelperOp::AvErr2Str(ret).c_str());
    return -1;
  }
  printf("> Wait.\n");
  ret = session->Wait();
  printf("< Wait. ret=%s\n", lalcc::HelperOp::AvErr2Str(ret).c_str());

  printf("bye. %d\n", chef::stuff_op::tick_msec());
  return 0;
}
