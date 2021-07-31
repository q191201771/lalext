// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _LALCC_HPP_
#define _LALCC_HPP_

#include "lalcc_rtmp_pull_session.hpp"

#endif

// int avformat_open_input(AVFormatContext **ps, const char *filename,
//                         ff_const59 AVInputFormat *fmt, AVDictionary **options)
//
//   <ps>可以为NULL，为NULL时内部会avformat_alloc_context()
//
//   如果返回值不为0，也即失败了，内部会释放<*ps>，并将它设置为NULL
//
//   对于rtmp拉流：
//     case1: -5
//     从rtmp服务器拉取一条不存在的流，即使收到了对端回复的play信令的结果，该函数依然不会返回，
//     直到对端超时关闭连接，该函数返回(-5:Input/output error)
//
//     case2: -5
//     使用interrupt_callback，在回调中主动关闭，该函数返回(-5:Input/output error)
//
//     case3: -111
//     连接本地端口没有监听的rtmp地址，该函数返回(-111:Connection refused)
//
//     TODO(chef): 详细跟一下内部实现

// AVFormatContext
//   nb_streams: 流的数量
//   streams:    流数组
//
// AVStream
//   codecpar: AVCodecParameters
//
// AVCodecParameters
//   codec_id:       相关函数 avcodec_get_name, avcodec_get_type, av_get_media_type_string
//   extradata:      rtmp h264: 跳过了前面5个字节的seq header
//                   rtmp aac:  跳过了前面2个字节的seq header(asc)
//   extradata_size
//
// AVPacket
//   stream_index: 对应AVStream以及AVStream上的AVCodecParameters
//   data:         rtmp h264: 是AVCC格式，跳过了rtmp body的前5个字节
//   size
//   dts
//

