// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _LALCC_HPP_
#define _LALCC_HPP_

#include "lalcc_av_packet_t.hpp"
#include "lalcc_forward_declaration.hpp"
#include "lalcc_helper_op.hpp"
#include "lalcc_log.hpp"
#include "lalcc_rtmp_pull_session.hpp"
#include "lalcc_rtmp_push_session.hpp"
#include "lalcc_pull_session.hpp"

#endif

// 以下对ffmpeg的接口和结构体做一些补充说明
// 更多的信息应从ffmpeg自身的注释说明以及源码实现中获取
//

// --------------------------------------------------------------------------------------------------------------------
// int avformat_open_input(AVFormatContext **ps, const char *filename,
//                         ff_const59 AVInputFormat *fmt, AVDictionary **options)
//
//   @param ps: 可以为NULL，为NULL时内部会alloc申请
//
//   @return 如果返回值不为0，也即失败了，内部会释放<*ps>，并将它设置为NULL
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
//

// --------------------------------------------------------------------------------------------------------------------
// void avformat_close_input(AVFormatContext **s);
//
//   @param s: 可以为NULL
//
//   内部会调用`avformat_free_context`
//
//   注意，avformat_open_input使用avformat_close_input释放；avformat_alloc_output_context2使用avformat_free_context释放

// --------------------------------------------------------------------------------------------------------------------
// void avformat_free_context(AVFormatContext *s);
//

// --------------------------------------------------------------------------------------------------------------------
// int avformat_alloc_output_context2(AVFormatContext **ctx, ff_const59 AVOutputFormat *oformat,
//                                    const char *format_name, const char *filename);
//
//   @param filename: 注意，当filename是rtmp url时，参数`format_name`必须指定为"flv"
//
//   使用rtmp推流时，调用该函数并不进行任何网络IO操作
//

// --------------------------------------------------------------------------------------------------------------------
// AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c);
//

// --------------------------------------------------------------------------------------------------------------------
// int avcodec_parameters_copy(AVCodecParameters *dst, const AVCodecParameters *src);
//
// 注意，这一步会拷贝extradata
//

// --------------------------------------------------------------------------------------------------------------------
// int avio_open(AVIOContext **s, const char *url, int flags);
//
// 注意，rtmp拉流时，到这一步才会发起TCP connect，并发送RTMP信令，直到发送RTMP publish信令
//

// --------------------------------------------------------------------------------------------------------------------
// int avformat_write_header(AVFormatContext *s, AVDictionary **options);
//
// 注意，rtmp拉流时，到这一步发送metadata
//
// TODO(chef): extradata啥时候发
//

// --------------------------------------------------------------------------------------------------------------------
// int av_read_frame(AVFormatContext *s, AVPacket *pkt);
//

// --------------------------------------------------------------------------------------------------------------------
// AVPacket *av_packet_alloc(void);
//   申请AVPacket结构体的内存，并做零值初始化，并不会申请buf内存
//
// void av_packet_unref(AVPacket *pkt);
//   释放buf内存的引用
//
// void av_packet_free(AVPacket **pkt);
//   内部会先unref再free结构体
//
// AVPacket *av_packet_clone(const AVPacket *src);
//   内部会先alloc再ref

// --------------------------------------------------------------------------------------------------------------------
// double av_q2d(AVRational a)
//
// av_ts2str(int64_t)
//
// int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq)
//
// int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq, enum AVRounding rnd)
//

// --------------------------------------------------------------------------------------------------------------------
//
// int av_write_frame(AVFormatContext *s, AVPacket *pkt);
//
// int av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt);
//

// --------------------------------------------------------------------------------------------------------------------
// int ff_avc_write_annexb_extradata(const uint8_t *in, uint8_t **buf, int *size);
//   将avcc格式的extradata(sps, pps)转换为annexb格式
//

// --------------------------------------------------------------------------------------------------------------------
// AVFormatContext
//   nb_streams: unsigned int 流的数量
//   streams:    流数组
//   oformat
//
//   interrupt_callback.callback
//   interrupt_callback.opaque
//     回调只和avformat_open_input阶段相关，avformat_open_input返回前，回调被持续调用，avformat_open_input返回后，回调不再被调用
//     所以这个方式只能用来做avformat_open_input的超时逻辑
//
//     case 1
//     拉取不存在的rtmp流，avformat_open_input没有返回，回调依然一直被调用
//
//     case 2
//     拉取存在的rtmp流，avformat_open_input返回成功后，回调不再被调用
//
//     TODO(chef): 跟ffmpeg的代码
//

// --------------------------------------------------------------------------------------------------------------------
// AVStream
//   index
//   codecpar: AVCodecParameters
//   time_base
//

// --------------------------------------------------------------------------------------------------------------------
// AVCodecParameters
//   codec_type:     enum AVMediaType 区分是音频还是视频
//   codec_id:       enum AVCodecID 编码类型
//   extradata:      rtmp h264: 跳过了前面5个字节的seq header
//                   rtmp aac:  跳过了前面2个字节的seq header(asc)
//   extradata_size
//   codec_tag:      case1: rtmp推流（包含h264+aac） 132160(0x20440)
//

// --------------------------------------------------------------------------------------------------------------------
// AVPacket
//   stream_index: 对应AVStream以及AVStream上的AVCodecParameters
//   data:         rtmp h264: 是AVCC格式，跳过了rtmp body的前5个字节
//   size
//   dts
//

// --------------------------------------------------------------------------------------------------------------------
// AVRational
//
