
#include "liblalc/lalc.h"

#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/timestamp.h"
#include "libavutil/opt.h"

struct LalcDecoder *audioDecoder = NULL;
struct LalcDecoder *audioDecoder2 = NULL;
struct LalcPcmFileWriter *pfw = NULL;
struct LalcPcmFileWriter *pfw2 = NULL;
struct LalcPcmFileWriter *pfw3 = NULL;
struct LalcAudioEncoder *audioEncoder = NULL;
struct LalcFrameList  *fl = NULL;

struct LalcVideoDecoder *videoDecoder = NULL;
struct LalcVideoEncoder *videoEncoder = NULL;
struct LalcFileWriter *fileWriter = NULL;

void onPullVideoPacket(AVFormatContext *fmtCtx, AVPacket *pkt) {
  LalcLogAvPacket(pkt);
  AVStream *stream = fmtCtx->streams[pkt->stream_index];
  // 转换annexb
//  printf("\n----- > packet\n");
  for (int i = 0; i != pkt->size;) {
    int nalLen =((int)(pkt->data[i]) << 24) +
    	((int)(pkt->data[i+1]) << 16) +
    	((int)(pkt->data[i+2]) << 8) +
    	((int)(pkt->data[i+3]));
    pkt->data[i] = 0;
    pkt->data[i+1] = 0;
    pkt->data[i+2] = 0;
    pkt->data[i+3] = 1;

//    printf("\n----- > nal\n");
//    printf("%d %d, %02x %02x %02x %02x, %d\n", pkt->size, nalLen, pkt->data[i], pkt->data[i+1], pkt->data[i+2], pkt->data[i+3], pkt->data[i+4] & 0x1f);
//    for (int j = 0; j < 8; j++) {
//      printf("%02x ", pkt->data[i+j]);
//    }
//    printf("\n----- < nal\n");

    i += 4;
    i += nalLen;
  }
  printf("\n----- < packet\n");

  // 解码
  int ret = LalcVideoDecoderSend(videoDecoder, pkt->data, pkt->size);
  if (ret < 0) {
    exit(0);
  }

  AVFrame *frame = av_frame_alloc();
  ret = LalcVideoDecoderTryReceive(videoDecoder, frame);
  printf("< receive. ret=%d, pict_type=%d\n", ret, frame->pict_type);
  if (frame->pict_type == AV_PICTURE_TYPE_NONE) {
    return;
  }

  // 写jpeg文件
//  char filename[1024] = {0};
//  static int count = 0;
//  snprintf(filename, 1023, "/tmp/%d.jpeg", count++);
//  LalcDumpMjpeg(frame, filename);

  // 编码
  AVPacket *encodePacket = av_packet_alloc();
  ret = LalcVideoEncoderSend(videoEncoder, frame);
  if (ret < 0) {
	return;
  }
  for (; ; ) {
    ret = LalcVideoEncoderTryReceive(videoEncoder, encodePacket);
    if (ret < 0) {
      break;
    }
	printf("encoded. %d %d, %02x %02x %02x %02x %02x %02x %02x %02x\n",
		   encodePacket->size, encodePacket->size,
		   encodePacket->data[0], encodePacket->data[1], encodePacket->data[2], encodePacket->data[3],
		   encodePacket->data[4], encodePacket->data[5], encodePacket->data[6], encodePacket->data[7]);

	LalcFileWriterWrite(fileWriter, encodePacket->data, encodePacket->size);
  }
}

void onPullAudioPacket(AVFormatContext *fmtCtx, AVPacket *pkt) {
  AVStream *stream = fmtCtx->streams[pkt->stream_index];
  printf("%d\n", stream->codecpar->channel_layout);
  AVFrame *frame = av_frame_alloc();

  LalcDecoderDecode(audioDecoder, pkt->data, pkt->size, frame);
  int ret = LalcPcmFileWriterWrite(pfw, frame);
  if (ret < 0) {
    exit(0);
  }

  AVPacket *encodePacket = av_packet_alloc();
  ret = LalcAudioEncoderEncode(audioEncoder, frame, encodePacket);
  if (ret < 0) {
    exit(0);
  }
  printf("encoded. %d %d %d\n", pkt->size, frame->linesize[0], encodePacket->size);
  if (encodePacket->size == 0) {
    return;
  }

  AVFrame *frame2 = av_frame_alloc();
  LalcDecoderDecode(audioDecoder2, encodePacket->data, encodePacket->size, frame2);
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

void onPullPacket(AVFormatContext *fmtCtx, AVPacket *pkt) {
  enum AVCodecID id = fmtCtx->streams[pkt->stream_index]->codecpar->codec_id;
  if (id == AV_CODEC_ID_AAC) {
	//onPullAudioPacket(fmtCtx, pkt);
  } else if (id == AV_CODEC_ID_H264) {
    onPullVideoPacket(fmtCtx, pkt);
  }
}

int main(int argc, char **argv) {
  const char*url = argv[1];
  int ret;

  // 音频
  audioDecoder = LalcDecoderAlloc();
  ret = LalcDecoderOpen(audioDecoder, 2, 48000);
  if (ret < 0) {
	exit(0);
  }

  audioDecoder2 = LalcDecoderAlloc();
  ret = LalcDecoderOpen(audioDecoder2, 2, 48000);
  if (ret < 0) {
	exit(0);
  }

  pfw = LalcPcmFileWriterAlloc();
  ret = LalcPcmFileWriterOpen(pfw, "/tmp/lalc_demo.pcm");
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

  audioEncoder = LalcAudioEncoderAlloc();
  ret = LalcAudioEncoderOpen(audioEncoder, 2, 48000);
  if (ret < 0) {
    exit(0);
  }

  fl = LalcFrameListAlloc(2);

  // 视频
  videoDecoder = LalcVideoDecoderAlloc();
  ret = LalcVideoDecoderOpen(videoDecoder);
  if (ret < 0) {
	exit(0);
  }

  videoEncoder = LalcVideoEncoderAlloc();
  ret = LalcVideoEncoderOpen(videoEncoder, 360, 640);
//  if (ret < 0) {
//	exit(0);
//  }

  fileWriter = LalcFileWriterAlloc();
  LalcFileWriterOpen(fileWriter, "/tmp/test.h264");
  if (ret < 0) {
	exit(0);
  }

  LalcOpPull(url, &onPullPacket);

  LalcDecoderRelease(audioDecoder);
  LalcPcmFileWriterRelease(pfw);
  LalcAudioEncoderRelease(audioEncoder);

  return 0;
}
