#ifndef _LALC_H_
#define _LALC_H_

#include <stdint.h>

#include <libavcodec/packet.h>
#include <libavutil/frame.h>
// 包含这两个文件用于提供AVPacket和AVFrame

// TODO(chef): [feat] LalcDecoderOpen with asc
// TODO(chef): [opt] 所有release增加NULL判断
// TODO(chef): [refactor] 全部typedef

typedef struct LalcDecoder       LalcDecoder;
typedef struct LalcAudioEncoder  LalcAudioEncoder;
typedef struct LalcPcmFileWriter LalcPcmFileWriter;
typedef struct LalcFrameList     LalcFrameList;
typedef struct LalcVideoDecoder  LalcVideoDecoder;
typedef struct LalcVideoEncoder  LalcVideoEncoder;

// ----- 音频解码 -------------------------------------------------------------------------------------------------------

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
// @param inSize: 输入数据。注意，传入裸数据即可，不需要ADTS头
//
// @param outFrame: 传入使用 av_frame_alloc() 生成的AVFrame。
//                  函数调用结束后，内部不再持有该AVFrame。
//                  上层可自由使用，并在不再需要时调用 av_frame_unref() 释放。
//
int LalcDecoderDecode(struct LalcDecoder *decoder, uint8_t *inData, int inSize, AVFrame *outFrame);

void LalcDecoderRelease(struct LalcDecoder *decoder);

// ----- 音频编码 -------------------------------------------------------------------------------------------------------

struct LalcAudioEncoder *LalcAudioEncoderAlloc();

int LalcAudioEncoderOpen(struct LalcAudioEncoder *encoder, int channels, int sampleRate);

int LalcAudioEncoderEncode(struct LalcAudioEncoder *encoder, AVFrame *frame, AVPacket *outPacket);

int LalcAudioEncoderRelease(struct LalcAudioEncoder*);

// ----- 音频PCM写文件 ---------------------------------------------------------------------------------------------------

struct LalcPcmFileWriter *LalcPcmFileWriterAlloc();

int LalcPcmFileWriterOpen(struct LalcPcmFileWriter *writer, char *filename);

int LalcPcmFileWriterWrite(struct LalcPcmFileWriter *writer, AVFrame *frame);

int LalcPcmFileWriterClose(struct LalcPcmFileWriter *writer);

int LalcPcmFileWriterRelease(struct LalcPcmFileWriter *writer);

// ----- 音频混音 -------------------------------------------------------------------------------------------------------

int LalcOpAudioMix(AVFrame **frameList, int frameListSize, AVFrame *outFrame);

LalcFrameList *LalcFrameListAlloc(int size);

void LalcFrameListAdd(LalcFrameList *list, AVFrame *frame);

void LalcFrameListClear(LalcFrameList *list);

int LalcFrameListRelease(LalcFrameList *list);

int LalcOpAudioMixWithFrameList(LalcFrameList *list, AVFrame *outFrame);

// ----- 视频解码 -------------------------------------------------------------------------------------------------------

struct LalcVideoDecoder *LalcVideoDecoderAlloc();

int LalcVideoDecoderOpen(struct LalcVideoDecoder *decoder);

int LalcVideoDecoderDecode(struct LalcVideoDecoder *decoder, uint8_t *inData, int inSize, AVFrame *outFrame);

void LalcVideoDecoderRelease(struct LalcVideoDecoder *decoder);

// ----- 视频编码 -------------------------------------------------------------------------------------------------------

struct LalcVideoEncoder *LalcVideoEncoderAlloc();

int LalcVideoEncoderOpen(struct LalcVideoEncoder *encoder, int channels, int sampleRate);

int LalcVideoEncoderEncode(struct LalcVideoEncoder *encoder, AVFrame frame, AVPacket *outPacket);

int LalcVideoEncoderRelease(struct LalcVideoEncoder);

// ----- 视频缩放 -------------------------------------------------------------------------------------------------------

int LalcVideoScale(AVFrame frame, int width, int height, AVFrame *outFrame);

// ----- 视频裁剪 -------------------------------------------------------------------------------------------------------

int LalcVideoCut(AVFrame frame, int x, int y, int width, int height, AVFrame *outFrame);

// ----- 视频拼接合并 ----------------------------------------------------------------------------------------------------

int LalcVideoMix(AVFrame *frameList, int frameListSize, int *xList, int *yList, AVFrame *bg);

// ----- 拉流 -----------------------------------------------------------------------------------------------------------

#include "libavformat/avformat.h"

typedef void (*LalcOpPullOnPacket)(AVFormatContext *fmtCtx, AVPacket *packet);

// LalcOpPull
//
// @param onPacket 收到数据的回调
//                 对于rtmp AAC，packet data中存储raw数据，不包含adts头，也不包含asc
//                 对于rtmp H264，packet data中存储avcc格式数据，不包含其他头
//
int LalcOpPull(const char *url, LalcOpPullOnPacket onPacket);

#endif