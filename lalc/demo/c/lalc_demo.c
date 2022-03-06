
#include "liblalc/lalc.h"

#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/timestamp.h"
#include "libavutil/opt.h"

struct LalcDecoder *decoder = NULL;
struct LalcDecoder *decoder2 = NULL;
struct LalcPcmFileWriter *pfw = NULL;
struct LalcPcmFileWriter *pfw2 = NULL;
struct LalcPcmFileWriter *pfw3 = NULL;
struct LalcAudioEncoder *encoder = NULL;
struct LalcFrameList  *fl = NULL;

struct LalcVideoDecoder *videoDecoder = NULL;

void onPullPacket(AVFormatContext *fmtCtx, AVPacket *pkt) {
  if (fmtCtx->streams[pkt->stream_index]->codecpar->codec_id == AV_CODEC_ID_H264) {
//    AVStream *stream = fmtCtx->streams[pkt->stream_index];
    AVFrame *frame = av_frame_alloc();

	printf("%d, %02x %02x %02x %02x\n", pkt->size, pkt->data[0], pkt->data[1], pkt->data[2], pkt->data[3]);
    LalcVideoDecoderDecode(videoDecoder, pkt->data, pkt->size, frame);
  }

  if (fmtCtx->streams[pkt->stream_index]->codecpar->codec_id == AV_CODEC_ID_AAC) {
    AVStream *stream = fmtCtx->streams[pkt->stream_index];
    printf("%d\n", stream->codecpar->channel_layout);
	AVFrame *frame = av_frame_alloc();

	LalcDecoderDecode(decoder, pkt->data, pkt->size, frame);
	int ret = LalcPcmFileWriterWrite(pfw, frame);
	if (ret < 0) {
	  exit(0);
	}

	AVPacket *encodePacket = av_packet_alloc();
	ret = LalcAudioEncoderEncode(encoder, frame, encodePacket);
	if (ret < 0) {
	  exit(0);
	}
	printf("encoded. %d %d %d\n", pkt->size, frame->linesize[0], encodePacket->size);
	if (encodePacket->size == 0) {
	  return;
	}

	AVFrame *frame2 = av_frame_alloc();
	LalcDecoderDecode(decoder2, encodePacket->data, encodePacket->size, frame2);
	LalcPcmFileWriterWrite(pfw2, frame2);
	av_frame_unref(frame2);

	AVFrame *frame3 = av_frame_alloc();
	LalcFrameListAdd(fl, frame);
	LalcFrameListAdd(fl, frame);
	ret = LalcOpAudioMixWithFrameList(fl, frame3);
	LalcFrameListClear(fl);
	if (ret < 0) {
	  exit(0);
	}
	LalcPcmFileWriterWrite(pfw3, frame3);
	av_frame_unref(frame3);

	av_packet_unref(encodePacket);

	av_frame_unref(frame);
  }

}

int main(int argc, char **argv) {
  const char*url = argv[1];

  decoder = LalcDecoderAlloc();
  LalcDecoderOpen(decoder, 2, 48000);

  decoder2 = LalcDecoderAlloc();
  LalcDecoderOpen(decoder2, 2, 48000);

  pfw = LalcPcmFileWriterAlloc();
  int ret = LalcPcmFileWriterOpen(pfw, "/tmp/lalc_demo.pcm");
  if (ret < 0) {
    exit(0);
  }

  pfw2 = LalcPcmFileWriterAlloc();
  ret = LalcPcmFileWriterOpen(pfw2, "/tmp/lalc_demo2.pcm");
  if (ret < 0) {
    exit(0);
  }

  pfw3 = LalcPcmFileWriterAlloc();
  ret = LalcPcmFileWriterOpen(pfw3, "/tmp/lalc_demo3.pcm");
  if (ret < 0) {
	exit(0);
  }

  encoder = LalcAudioEncoderAlloc();
  ret = LalcAudioEncoderOpen(encoder, 2, 48000);
  if (ret < 0) {
    exit(0);
  }

  fl = LalcFrameListAlloc(2);

  videoDecoder = LalcVideoDecoderAlloc();
  LalcVideoDecoderOpen(videoDecoder);

  LalcOpPull(url, &onPullPacket);

  LalcDecoderRelease(decoder);
  LalcPcmFileWriterRelease(pfw);
  LalcAudioEncoderRelease(encoder);

  return 0;
}