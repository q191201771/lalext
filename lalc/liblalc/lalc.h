#ifndef _LALC_H_
#define _LALC_H_

#include <stdint.h>

#include <libavcodec/packet.h>
#include <libavutil/frame.h>
// 包含这两个文件用于提供AVPacket和AVFrame

// TODO(chef): [feat] LalcDecoderOpen with asc
// TODO(chef): [opt] 所有release增加NULL判断
// TODO(chef): [refactor] 全部typedef

// ----- 1. 音频解码 ---------------------------------------------------------------------------------------------------

typedef struct LalcDecoder LalcDecoder;

// LalcDecoderAlloc
//
struct LalcDecoder *LalcDecoderAlloc();

// LalcDecoderOpen
//
// @param channels: 声道数，比如2
//
// @param sampleRate: 采样率，比如48000
//
int LalcDecoderOpen(struct LalcDecoder *decoder, int channels, int sampleRate);

// LalcDecoderDecode
//
// @param inData: TODO(chef): 内部是否引用inData
//
// @param inSize: 输入数据。格式是裸数据，不需要ADTS头
//
// @param outFrame:
//  用于接收解码后的数据。
//
//  传入 av_frame_alloc() 生成的AVFrame。
//
//  函数调用结束后，内部不再持有该AVFrame。
//  上层可自由使用，并在不再需要时调用 av_frame_unref() 释放。
//
int LalcDecoderDecode(struct LalcDecoder *decoder, uint8_t *inData, int inSize, AVFrame *outFrame);

// LalcDecoderRelease
//
void LalcDecoderRelease(struct LalcDecoder *decoder);

// ----- 2. 音频编码 ---------------------------------------------------------------------------------------------------

typedef struct LalcAudioEncoder  LalcAudioEncoder;

// LalcAudioEncoderAlloc
//
struct LalcAudioEncoder *LalcAudioEncoderAlloc();

// LalcAudioEncoderOpen
//
int LalcAudioEncoderOpen(struct LalcAudioEncoder *encoder, int channels, int sampleRate);

// LalcAudioEncoderEncode
//
// @param outPacket: 输出数据。格式是裸数据，不需要adts头
//
int LalcAudioEncoderEncode(struct LalcAudioEncoder *encoder, AVFrame *frame, AVPacket *outPacket);

// LalcAudioEncoderRelease
//
void LalcAudioEncoderRelease(struct LalcAudioEncoder *encoder);

// ----- 3. 音频混音 ---------------------------------------------------------------------------------------------------

typedef struct LalcFrameList LalcFrameList;

// LalcOpAudioMix
//
// 混音方式一
//
// @param frameList: 输入，AVFrame* 数组
//
// @param frameListSize: 数组大小
//
// @param outFrame: 输出，混音后数据
//
int LalcOpAudioMix(AVFrame **frameList, int frameListSize, AVFrame *outFrame);

// LalcFrameListAlloc LalcFrameListAdd LalcFrameListClear LalcFrameListRelease
//
// 构建管理 LalcFrameList
//
LalcFrameList *LalcFrameListAlloc(int size);

void LalcFrameListAdd(LalcFrameList *list, AVFrame *frame);

void LalcFrameListClear(LalcFrameList *list);

int LalcFrameListRelease(LalcFrameList *list);

// LalcOpAudioMixWithFrameList
//
// 混音方式二，使用 LalcFrameList 作为输入
//
int LalcOpAudioMixWithFrameList(LalcFrameList *list, AVFrame *outFrame);


// ----- 4. 视频解码 --------------------------------------------------------------------------------------------------

typedef struct LalcVideoDecoder  LalcVideoDecoder;

// LalcVideoDecoderAlloc
//
struct LalcVideoDecoder *LalcVideoDecoderAlloc();

// LalcVideoDecoderOpen
//
int LalcVideoDecoderOpen(struct LalcVideoDecoder *decoder);

// LalcVideoDecoderSend
//
// @param inData:
//  annexb格式
//
//  验证可行的输入顺序:
//  - (sps pps I) (P) (P) ...
//
//
int LalcVideoDecoderSend(struct LalcVideoDecoder *decoder, uint8_t *inData, int inSize);

// LalcVideoDecoderTryReceive
//
// 尝试获取解码后的数据
//
// @return:
//  - 如果返回0，表示获取已解码数据成功，调用方应继续循环获取
//  - 如果返回-35，表示当前没有已解码数据，应在下次LalcVideoDecoderSend之后调用LalcVideoDecoderTryReceive
//  - 如果返回其他值，表示解码发生错误
//
// @param outFrame:
//  用于接收解码后的数据。
//  传入 av_frame_alloc() 生成的AVFrame。
//  函数调用结束后，内部不再持有该AVFrame。
//  上层可自由使用，并在不再需要时调用 av_frame_unref() 释放。
//
int LalcVideoDecoderTryReceive(struct LalcVideoDecoder *decoder, AVFrame *outFrame);

// LalcVideoDecoderRelease
//
void LalcVideoDecoderRelease(struct LalcVideoDecoder *decoder);

// ----- 5. 视频编码 ---------------------------------------------------------------------------------------------------

typedef struct LalcVideoEncoder LalcVideoEncoder;

// LalcVideoEncoderAlloc
//
struct LalcVideoEncoder *LalcVideoEncoderAlloc();

// LalcVideoEncoderOpen
//
int LalcVideoEncoderOpen(struct LalcVideoEncoder *encoder, int width, int height);

// LalcVideoEncoderSend
//
int LalcVideoEncoderSend(struct LalcVideoEncoder *encoder, AVFrame *frame);

// LalcVideoEncoderTryReceive
//
// @param outPacket:
//  annexb格式
//
//  输出顺序是: (sps pps I) (P) (P)...
//
int LalcVideoEncoderTryReceive(struct LalcVideoEncoder *encoder, AVPacket *outPacket);

// LalcVideoEncoderRelease
//
void LalcVideoEncoderRelease(struct LalcVideoEncoder *encoder);

// ----- 6. 视频拼接合并 ----------------------------------------------------------------------------------------------

// LalcVideoPin
//
int LalcVideoPin(AVFrame *bg, AVFrame *part, int x, int y);

// ----- 视频缩放 -----------------------------------------------------------------------------------------------------

int LalcVideoScale(AVFrame frame, int width, int height, AVFrame *outFrame);

// ----- 视频裁剪 -------------------------------------------------------------------------------------------------------

int LalcVideoCut(AVFrame frame, int x, int y, int width, int height, AVFrame *outFrame);


// ----- 音频PCM写文件 ---------------------------------------------------------------------------------------------------

// LalcPcmFileWriter
//
// ffplay -ar 48000 -channels 2 -f f32le -i demo.pcm
//
typedef struct LalcPcmFileWriter LalcPcmFileWriter;

struct LalcPcmFileWriter *LalcPcmFileWriterAlloc();

int LalcPcmFileWriterOpen(struct LalcPcmFileWriter *writer, char *filename);

int LalcPcmFileWriterWrite(struct LalcPcmFileWriter *writer, AVFrame *frame);

int LalcPcmFileWriterClose(struct LalcPcmFileWriter *writer);

int LalcPcmFileWriterRelease(struct LalcPcmFileWriter *writer);

// TODO(chef): 直接写YUV420P文件

// ----- 写JPEG文件 -----------------------------------------------------------------------------------------------------

int LalcDumpMjpeg(AVFrame *frame, const char *filename);

// ----- 写文件 ---------------------------------------------------------------------------------------------------------

typedef struct LalcFileWriter LalcFileWriter;

struct LalcFileWriter *LalcFileWriterAlloc();

int LalcFileWriterOpen(struct LalcFileWriter *writer, char *filename);

int LalcFileWriterWrite(struct LalcFileWriter *writer,  const void *data, int size);

int LalcFileWriterClose(struct LalcFileWriter *writer);

int LalcFileWriterRelease(struct LalcFileWriter *writer);

// ----- 拉流 -----------------------------------------------------------------------------------------------------------

#include "libavformat/avformat.h"

typedef void (*LalcOpPullOnPacket)(AVFormatContext *fmtCtx, AVPacket *packet);

// LalcOpPull
//
// @param onPacket 收到数据的回调
//  - 对于rtmp AAC，packet data中存储raw数据，不包含adts头，也不包含asc
//  - 对于rtmp H264，packet data中存储avcc格式([len+nal...])数据，不包含其他头，也不包含vsh，非vsh的rtmp message按原样返回
//    rtmp H264实例如下：
//     - case 1:
//       rtmp流实际message格式为: (sh: sps, pps) (sps, pps, I) (P) (P)
//       对应的返回avpacket格式: (sps, pps, I) (P) (P)
//     - case 2:
//       rtmp流实际message格式为: (sh: sps, pps) (I) (P) (P)
//       对应的返回avpacket格式: (I) (P) (P)
//     - case 3:
//       rtmp流实际message格式为: (sh: sps, pps) (sps, pps, I) (P) (P)
//       对应的返回avpacket格式: (sps, pps, I) (P) (P)
//
int LalcOpPull(const char *url, LalcOpPullOnPacket onPacket);

// --------------------------------------------------------------------------------------------------------------------

void LalcLogAvFrame(AVFrame *frame);
void LalcLogAvPacket(AVPacket *packet);

#endif
