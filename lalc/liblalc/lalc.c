#include "lalc.h"

#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/timestamp.h"
#include "libavutil/opt.h"

#include <stdlib.h>

#define PRINT_AV_ERROR(errnum) { \
char b[AV_ERROR_MAX_STRING_SIZE] = {0}; \
av_make_error_string(b, AV_ERROR_MAX_STRING_SIZE, (errnum)); \
printf("[AVERROR] %d(%s) %s:%d", errnum, b, __FUNCTION__, __LINE__); \
}

// ---------------------------------------------------------------------------------------------------------------------

typedef struct LalcDecoder {
  AVCodecContext *codecCtx_;
}LalcDecoder;

struct LalcDecoder *LalcDecoderAlloc() {
  LalcDecoder *decoder = (LalcDecoder *)malloc(sizeof(LalcDecoder));
  decoder->codecCtx_ = NULL;
  return decoder;
}

int LalcDecoderOpen(struct LalcDecoder *decoder, int channels, int sampleRate) {
  enum AVCodecID codecId = AV_CODEC_ID_AAC;
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
  param->codec_type = AVMEDIA_TYPE_AUDIO;
  param->channels = channels;
  param->sample_rate = sampleRate;

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
  printf("> LalcDecoderDecode. inSize=%d, %d %d\n", inSize, (int)inData[0], (int)inData[1]);
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
  if (decoder->codecCtx_) {
	avcodec_free_context(decoder->codecCtx_);
	decoder->codecCtx_ = NULL;
  }
  free(decoder);
}

// ---------------------------------------------------------------------------------------------------------------------

typedef struct LalcAudioEncoder {
  AVCodecContext *codecCtx_;
}LalcAudioEncoder;

struct LalcAudioEncoder *LalcAudioEncoderAlloc() {
  LalcAudioEncoder *encoder = (LalcAudioEncoder *)malloc(sizeof(LalcAudioEncoder));
  encoder->codecCtx_ = NULL;
  return encoder;
}

int LalcAudioEncoderOpen(struct LalcAudioEncoder *encoder, int channels, int sampleRate) {
  enum AVCodecID codecId = AV_CODEC_ID_AAC;
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

  AVCodecParameters *param = avcodec_parameters_alloc();
  param->codec_type = AVMEDIA_TYPE_AUDIO;
  param->channels = channels;
  param->sample_rate = sampleRate;

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
  AVPacket *packet = av_packet_alloc();

  int ret = avcodec_send_frame(encoder->codecCtx_, frame);
  if (ret < 0) {
    PRINT_AV_ERROR(ret);
    return ret;
  }

  ret = avcodec_receive_packet(encoder->codecCtx_, outPacket);
  if (ret < 0) {
    if (ret == AVERROR(EAGAIN)) {
      return 0;
    }
    PRINT_AV_ERROR(ret);
    return ret;
  }

  return 0;
}

int LalcAudioEncoderRelease(struct LalcAudioEncoder *encoder) {
	if (encoder->codecCtx_) {
	  avcodec_free_context(encoder->codecCtx_);
	  encoder->codecCtx_ = NULL;
	}
	free(encoder);
}

// ---------------------------------------------------------------------------------------------------------------------

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

  printf("LalcPcmFileWriterWrite. %d %d %d\n", nbSamples, channels, sampleSize);

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