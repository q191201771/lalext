#include "lalc.h"

#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/timestamp.h"
#include "libavutil/opt.h"

#include <stdlib.h>

// ---------------------------------------------------------------------------------------------------------------------

#define PRINT_AV_ERROR(errnum) { \
char b[AV_ERROR_MAX_STRING_SIZE] = {0}; \
av_make_error_string(b, AV_ERROR_MAX_STRING_SIZE, (errnum)); \
printf("[AVERROR] %d(%s) %s:%d\n", errnum, b, __FUNCTION__, __LINE__); \
};

// ----- 1. 音频解码 ----------------------------------------------------------------------------------------------------

typedef struct LalcDecoder {
  AVCodecContext *codecCtx_;
} LalcDecoder;

struct LalcDecoder *LalcDecoderAlloc() {
  LalcDecoder *decoder = (LalcDecoder *)malloc(sizeof(LalcDecoder));
  decoder->codecCtx_ = NULL;
  return decoder;
}

int LalcDecoderOpen(struct LalcDecoder *decoder, int channels, int sampleRate) {
  enum AVCodecID codecId = AV_CODEC_ID_AAC;
  AVCodecParameters *param = avcodec_parameters_alloc();
  param->codec_type = AVMEDIA_TYPE_AUDIO;
  param->channels = channels;
  param->sample_rate = sampleRate;

  int ret;

  AVCodec *codec = avcodec_find_decoder(codecId);
  if (!codec) {
    printf("[AVERROR] avcodec_find_decoder failed.");
    return -1;
  }

  decoder->codecCtx_ = avcodec_alloc_context3(codec);
  if (!decoder->codecCtx_) {
    printf("[AVERROR] avcodec_alloc_context3 failed.");
    return -1;
  }

  ret = avcodec_parameters_to_context(decoder->codecCtx_, param);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }

  ret = avcodec_open2(decoder->codecCtx_, codec, NULL);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }

  return 0;
}

int LalcDecoderDecode(struct LalcDecoder *decoder, uint8_t *inData, int inSize, AVFrame *outFrame) {
//  printf("> LalcDecoderDecode. inSize=%d, %d %d\n", inSize, (int)inData[0], (int)inData[1]);
  AVPacket *packet = av_packet_alloc();
  packet->data = inData;
  packet->size = inSize;

  int ret = avcodec_send_packet(decoder->codecCtx_, packet);
  if (ret < 0) {
	PRINT_AV_ERROR(ret);
    return ret;
  }

  ret = avcodec_receive_frame(decoder->codecCtx_, outFrame);
  if (ret < 0) {
    if (ret == AVERROR(EAGAIN)) {
      return 0;
    }
    PRINT_AV_ERROR(ret);
    return ret;
  }

  return 0;
}

void LalcDecoderRelease(struct LalcDecoder *decoder) {
  if (!decoder) { return; }

  if (decoder->codecCtx_) {
	avcodec_free_context(&decoder->codecCtx_);
	decoder->codecCtx_ = NULL;
  }
  free(decoder);
}

// ----- 音频编码 -------------------------------------------------------------------------------------------------------

typedef struct LalcAudioEncoder {
  AVCodecContext *codecCtx_;
} LalcAudioEncoder;

struct LalcAudioEncoder *LalcAudioEncoderAlloc() {
  LalcAudioEncoder *encoder = (LalcAudioEncoder *)malloc(sizeof(LalcAudioEncoder));
  encoder->codecCtx_ = NULL;
  return encoder;
}

int LalcAudioEncoderOpen(struct LalcAudioEncoder *encoder, int channels, int sampleRate) {
  enum AVCodecID codecId = AV_CODEC_ID_AAC;
  // TODO(chef): [fix] free me
  AVCodecParameters *param = avcodec_parameters_alloc();
  param->codec_type = AVMEDIA_TYPE_AUDIO;
  param->channels = channels;
  param->sample_rate = sampleRate;

  param->format = AV_SAMPLE_FMT_FLTP;
  param->channel_layout = AV_CH_FRONT_LEFT | AV_CH_FRONT_RIGHT;

  int ret;

  AVCodec *codec = avcodec_find_encoder(codecId);
  if (!codec) {
    printf("[AVERROR] avcodec_find_encoder failed.");
    return -1;
  }

  encoder->codecCtx_ = avcodec_alloc_context3(codec);
  if (!encoder->codecCtx_) {
    printf("[AVERROR] avcodec_alloc_context3 failed.");
    return -1;
  }

  ret = avcodec_parameters_to_context(encoder->codecCtx_, param);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }

  ret = avcodec_open2(encoder->codecCtx_, codec, NULL);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }

  return 0;
}

int LalcAudioEncoderEncode(struct LalcAudioEncoder *encoder, AVFrame *frame, AVPacket *outPacket) {
  int ret = avcodec_send_frame(encoder->codecCtx_, frame);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }

  ret = avcodec_receive_packet(encoder->codecCtx_, outPacket);
  if (ret < 0) {
    if (ret == AVERROR(EAGAIN)) {
	  PRINT_AV_ERROR(ret);
      return 0;
    }
    PRINT_AV_ERROR(ret);
    return ret;
  }

  return 0;
}

void LalcAudioEncoderRelease(struct LalcAudioEncoder *encoder) {
  if (!encoder) { return; }

  if (encoder->codecCtx_) {
	avcodec_free_context(&encoder->codecCtx_);
	encoder->codecCtx_ = NULL;
  }
  free(encoder);
}

// ----- 音频PCM写文件 ---------------------------------------------------------------------------------------------------

typedef struct LalcPcmFileWriter {
  FILE *fp;
} LalcPcmFileWriter;

struct LalcPcmFileWriter *LalcPcmFileWriterAlloc() {
  LalcPcmFileWriter *writer = (struct LalcPcmFileWriter *)malloc(sizeof(LalcPcmFileWriter));
  writer->fp = NULL;
  return writer;
}

int LalcPcmFileWriterOpen(struct LalcPcmFileWriter *writer, char *filename) {
  writer->fp = fopen(filename, "wb+");
  if (!writer->fp) {
    printf("[AVERROR] fopen failed.\n");
    return -1;
  }
  return 0;
}

int LalcPcmFileWriterWrite(struct LalcPcmFileWriter *writer, AVFrame *frame) {
  // libavutil/samplefmt.h

  int nbSamples = frame->nb_samples;
  int channels = frame->channels;
  int sampleSize = av_get_bytes_per_sample(frame->format);

  //  printf("LalcPcmFileWriterWrite. %d %d %d\n", nbSamples, channels, sampleSize);

  int isplanar = av_sample_fmt_is_planar(frame->format);

  if (isplanar) {
    //存储方式1 只存一个通道
    //	 for (int i = 0; i < frame->nb_samples; i++) {
    //	   fwrite(frame->data[0]+sampleSize * i, sampleSize, 1, writer->fp);
    //	 }

    // 存储方式2 存两个通道。planar转换成packed存储
    for (int i = 0; i < nbSamples; i++) {
      for (int j = 0; j < channels; j++) {
        fwrite(frame->data[j]+sampleSize * i, sampleSize, 1, writer->fp);
      }
    }

    fflush(writer->fp);
  }

  return 0;
}

int LalcPcmFileWriterClose(struct LalcPcmFileWriter *writer) {
  if (writer->fp) {
    int ret = fclose(writer->fp);
    writer->fp = NULL;
    return ret;
  }
  return 0;
}

int LalcPcmFileWriterRelease(struct LalcPcmFileWriter *writer) {
  LalcPcmFileWriterClose(writer);
  free(writer);
}

// ----- 音频混音 -------------------------------------------------------------------------------------------------------

int LalcOpAudioMix(AVFrame **frameList, int frameListSize, AVFrame *outFrame) {
  AVFrame *frame = frameList[0];
  int nbSamples = frame->nb_samples;
  int channels = frame->channels;
  int sampleSize = av_get_bytes_per_sample(frame->format);
  int isplanar = av_sample_fmt_is_planar(frame->format);

  outFrame->nb_samples = frame->nb_samples;
  outFrame->channels = frame->channels;
  outFrame->channel_layout = frame->channel_layout;
  outFrame->sample_rate = frame->sample_rate;
  outFrame->format = frame->format;
  if (isplanar) {
	outFrame->linesize[0] = frame->linesize[0];
    for (int i = 0; i < channels; i++) {
	  outFrame->data[i] = av_malloc(outFrame->linesize[0]);
	  memcpy(outFrame->data[i], frame->data[i], outFrame->linesize[0]);
    }
  }

  for (int i = 0; i < nbSamples; i++) {
    for (int j = 0; j < channels; j++) {
      float *dest = (float *)(outFrame->data[j] + sampleSize + i);
      *dest = 0;
	  for (int k = 0; k < frameListSize; k++) {
	    float *src = (float *)(frameList[k]->data[j] + sampleSize + i);
//	    printf("%0.2f\n", *src);
	    *dest += *src;
      }
	  *dest /= frameListSize;
    }
  }
}

typedef struct LalcFrameList {
  AVFrame **list;
  int size;
  int pos;
} LalcFrameList;

LalcFrameList *LalcFrameListAlloc(int size) {
  LalcFrameList *fl = (LalcFrameList *)malloc(sizeof(struct LalcFrameList));
  fl->list = (AVFrame **)malloc(sizeof(AVFrame *) * size);
  fl->size = size;
  fl->pos = 0;
  return fl;
}

void LalcFrameListAdd(LalcFrameList *list, AVFrame *frame) {
  list->list[list->pos++] = frame;
}

void LalcFrameListClear(LalcFrameList *list) {
  list->pos = 0;
}

int LalcFrameListRelease(LalcFrameList *list) {
  free(list->list);
  free(list);
}

int LalcOpAudioMixWithFrameList(LalcFrameList *list, AVFrame *outFrame) {
  LalcOpAudioMix(list->list, list->size, outFrame);
}

// ----- 视频解码 -------------------------------------------------------------------------------------------------------

typedef struct LalcVideoDecoder {
  AVCodecContext *codecCtx_;
} LalcVideoDecoder;

struct LalcVideoDecoder *LalcVideoDecoderAlloc() {
  LalcVideoDecoder *decoder = (LalcVideoDecoder *)malloc(sizeof(LalcVideoDecoder));
  decoder->codecCtx_ = NULL;
  return decoder;
}

int LalcVideoDecoderOpen(struct LalcVideoDecoder *decoder) {
  enum AVCodecID codecId = AV_CODEC_ID_H264;
  int ret;

  AVCodec *codec = avcodec_find_decoder(codecId);
  if (!codec) {
    printf("[AVERROR] avcodec_find_decoder failed.");
    return -1;
  }

  decoder->codecCtx_ = avcodec_alloc_context3(codec);
  if (!decoder->codecCtx_) {
    printf("[AVERROR] avcodec_alloc_context3 failed.");
    return -1;
  }

  AVCodecParameters *param = avcodec_parameters_alloc();
  param->codec_type = AVMEDIA_TYPE_VIDEO;
  param->format = AV_PIX_FMT_YUV420P;

  ret = avcodec_parameters_to_context(decoder->codecCtx_, param);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }

  ret = avcodec_open2(decoder->codecCtx_, codec, NULL);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }

  return 0;
}

int LalcVideoDecoderSend(struct LalcVideoDecoder *decoder, uint8_t *inData, int inSize) {
  AVPacket *packet = av_packet_alloc();
  packet->data = inData;
  packet->size = inSize;

  int ret = avcodec_send_packet(decoder->codecCtx_, packet);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }
}

int LalcVideoDecoderTryReceive(struct LalcVideoDecoder *decoder, AVFrame *outFrame) {
  int ret = avcodec_receive_frame(decoder->codecCtx_, outFrame);
  if (ret < 0) {
    if (ret == AVERROR(EAGAIN)) {
      // 依然返回错误，供调用方判断
      return ret;
    }
    PRINT_AV_ERROR(ret);
    return ret;
  }

  return 0;
}

void LalcVideoDecoderRelease(struct LalcVideoDecoder *decoder) {

}

// ----- 视频编码 -------------------------------------------------------------------------------------------------------

typedef struct LalcVideoEncoder {
  AVCodecContext *codecCtx_;
} LalcVideoEncoder;

struct LalcVideoEncoder *LalcVideoEncoderAlloc() {
	LalcVideoEncoder *encoder = (LalcVideoEncoder *)malloc(sizeof(LalcVideoEncoder));
	encoder->codecCtx_ = NULL;
	return encoder;
}

int LalcVideoEncoderOpen(struct LalcVideoEncoder *encoder, int width, int height) {
  enum AVCodecID codecId = AV_CODEC_ID_H264;
  // TODO(chef): [fix] free me
  AVCodecParameters *param = avcodec_parameters_alloc();
  param->codec_type = AVMEDIA_TYPE_VIDEO;
  param->width = width;
  param->height = height;
  param->format = AV_PIX_FMT_YUV420P;

  AVRational timebase;
  timebase.num = 1;
  timebase.den = 1000;

  int ret;

  AVCodec *codec = avcodec_find_encoder(codecId);
  if (!codec) {
    printf("[AVERROR] avcodec_find_encoder failed.");
    return -1;
  }

  encoder->codecCtx_ = avcodec_alloc_context3(codec);
  if (!encoder->codecCtx_) {
    printf("[AVERROR] avcodec_alloc_context3 failed.");
    return -1;
  }

  ret = avcodec_parameters_to_context(encoder->codecCtx_, param);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }
  encoder->codecCtx_->time_base = timebase;

  ret = avcodec_open2(encoder->codecCtx_, codec, NULL);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }

  return 0;
}

int LalcVideoEncoderSend(struct LalcVideoEncoder *encoder, AVFrame *frame) {
  printf("> LalcVideoEncoderSend\n");
  int ret = avcodec_send_frame(encoder->codecCtx_, frame);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
  }
  return ret;
}


int LalcVideoEncoderTryReceive(struct LalcVideoEncoder *encoder, AVPacket *outPacket) {
  printf("> LalcVideoEncoderTryReceive\n");
  int ret = avcodec_receive_packet(encoder->codecCtx_, outPacket);
  if (ret < 0) {
    if (ret == AVERROR(EAGAIN)) {
      return ret;
    }
    PRINT_AV_ERROR(ret);
    return ret;
  }

  return 0;
}

void LalcVideoEncoderRelease(struct LalcVideoEncoder *encoder) {
  if (!encoder) { return; }

  if (encoder->codecCtx_) {
	avcodec_free_context(&encoder->codecCtx_);
	encoder->codecCtx_ = NULL;
  }
  free(encoder);
}

// ----- 写JPEG文件 -----------------------------------------------------------------------------------------------------

int LalcDumpMjpeg(AVFrame *frame, const char *filename) {
  int ret = 0;

  AVFormatContext *fmt_ctx = NULL;
  AVCodec *codec = NULL;
  AVStream *stream = NULL;
  AVCodecContext *cc = NULL;
  AVPacket packet;
  enum AVCodecID codec_id = AV_CODEC_ID_MJPEG;

  ret = avformat_alloc_output_context2(&fmt_ctx, NULL, NULL, filename);
  if (ret < 0) {
	PRINT_AV_ERROR(ret);
    goto END;
  }

  fmt_ctx->oformat->video_codec = codec_id;

  codec = avcodec_find_encoder(codec_id);
  if (!codec) {
    printf("[AVERROR] avcodec_find_encoder failed.");
    ret = -1;
    goto END;
  }

  stream = avformat_new_stream(fmt_ctx, NULL);
  if (!stream) {
    printf("[AVERROR] avformat_new_stream failed.");
	ret = -1;
    goto END;
  }

  stream->id = 0;

  cc = avcodec_alloc_context3(codec);
  if (!cc) {
    printf("[AVERROR] avcodec_alloc_context3 failed.");
	ret = -1;
    goto END;
  }

  cc->codec_id = codec_id;
  cc->codec_type = AVMEDIA_TYPE_VIDEO;
  cc->pix_fmt = AV_PIX_FMT_YUVJ420P;
  cc->width = frame->width;
  cc->height = frame->height;
  cc->flags |= AV_CODEC_FLAG_QSCALE;
  cc->global_quality = FF_QP2LAMBDA * 1;

  stream->time_base = (AVRational){1, 25};
  cc->time_base = stream->time_base;

  ret = avcodec_open2(cc, codec, NULL);
  if (ret < 0) {
	PRINT_AV_ERROR(ret);
	goto END;
  }

  ret = avcodec_parameters_from_context(stream->codecpar, cc);
  if (ret < 0) {
	PRINT_AV_ERROR(ret);
    goto END;
  }

  ret = avformat_write_header(fmt_ctx, NULL);
  if (ret < 0) {
	PRINT_AV_ERROR(ret);
    goto END;
  }

  av_new_packet(&packet, cc->width * cc->height * 4);

  ret = avcodec_send_frame(cc, frame);
  if (ret < 0) {
	PRINT_AV_ERROR(ret);
    goto END;
  }

  while (ret >= 0) {
    ret = avcodec_receive_packet(cc, &packet);
    if (ret < 0) {
      if (ret != AVERROR(EAGAIN)) {
		PRINT_AV_ERROR(ret);
      }
      goto END;
    }
    ret = av_interleaved_write_frame(fmt_ctx, &packet);
    if (ret < 0) {
	  PRINT_AV_ERROR(ret);
      goto END;
    }
  }

  av_write_trailer(fmt_ctx);

  ret = 0;
  goto END;

  END:
  if (cc) { avcodec_close(cc); }
  if (fmt_ctx) { avformat_free_context(fmt_ctx); }

  return ret;
}

// ----- 写文件 ---------------------------------------------------------------------------------------------------------

typedef struct LalcFileWriter {
  FILE *fp;
} LalcFileWriter;

struct LalcFileWriter *LalcFileWriterAlloc() {
  LalcFileWriter *writer = (struct LalcFileWriter *)malloc(sizeof(LalcFileWriter));
  writer->fp = NULL;
  return writer;
}

int LalcFileWriterOpen(struct LalcFileWriter *writer, char *filename) {
  writer->fp = fopen(filename, "wb+");
  if (!writer->fp) {
    printf("[AVERROR] fopen failed.\n");
    return -1;
  }
  return 0;
}

int LalcFileWriterWrite(struct LalcFileWriter *writer,  const void *data, int size) {
  fwrite(data, size, 1, writer->fp);
  fflush(writer->fp);
  return 0;
}

int LalcFileWriterClose(struct LalcFileWriter *writer) {
  if (writer->fp) {
    int ret = fclose(writer->fp);
    writer->fp = NULL;
    return ret;
  }
  return 0;
}

int LalcFileWriterRelease(struct LalcFileWriter *writer) {
  LalcPcmFileWriterClose(writer);
  free(writer);
}


// ----- 拉流 -----------------------------------------------------------------------------------------------------------

int LalcOpPull(const char *url, LalcOpPullOnPacket onPacket) {
  AVFormatContext *fmtCtx_;
  fmtCtx_ = avformat_alloc_context();
  AVDictionary *options=NULL;
  int ret = avformat_open_input(&fmtCtx_, url, NULL, &options);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }

  ret = avformat_find_stream_info(fmtCtx_, NULL);
  if (ret < 0) {
	PRINT_AV_ERROR(ret);
    return ret;
  }

  for (; ; ) {
    AVPacket *pkt = av_packet_alloc();

    ret = av_read_frame(fmtCtx_, pkt);
    if (ret < 0) {
	  av_packet_free(&pkt);
	  PRINT_AV_ERROR(ret);
	  return ret;
    }
    onPacket(fmtCtx_, pkt);

    av_packet_free(&pkt);
  }
}

int LalcVideoScale(AVFrame frame, int width, int height, AVFrame *outFrame) {
  return 0;
}

int LalcVideoCut(AVFrame frame, int x, int y, int width, int height, AVFrame *outFrame) {
  return 0;
}

int LalcVideoMix(AVFrame *frameList, int frameListSize, int *xList, int *yList, AVFrame *bg) {
  return 0;
}