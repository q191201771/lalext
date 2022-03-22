package main

//#cgo CFLAGS: -I../../liblalc
//#cgo LDFLAGS: -L${SRCDIR}/../../output -llalc -lavfilter -lavformat -lavdevice -lavcodec -lavutil -lpostproc -lswresample -lswscale -lx264 -lx265 -lfdk-aac -lopus -lm -lz -ldl
//
//#include "lalc.h"
//
import "C"
import (
	"github.com/q191201771/lal/pkg/aac"
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/remux"
	"github.com/q191201771/lal/pkg/rtmp"
	"github.com/q191201771/naza/pkg/nazalog"
	"os"
	"sync"
	"time"
)

var (
	ic1 inCtx
	ic2 inCtx

	oc outCtx
)

// ---------------------------------------------------------------------------------------------------------------------

// inCtx
//
// 输入rtmp 拉流url，得到解码后的音频、视频数据
//
type inCtx struct {
	session *rtmp.PullSession

	audioDecoder *C.LalcDecoder

    videoDecoder *C.LalcVideoDecoder
	spspps []byte
}

func (ic *inCtx) Pull(url string, onAudio func(frame *C.AVFrame), onVideo func(frame *C.AVFrame)) error {
	// CHEFGREPME 1. 拉流
	ic.session = rtmp.NewPullSession()

	ic.audioDecoder = C.LalcDecoderAlloc()
	ret := C.LalcDecoderOpen(ic.audioDecoder, 2, 48000)
	nazalog.Assert(0, int(ret))

	ic.videoDecoder = C.LalcVideoDecoderAlloc()
    ret = C.LalcVideoDecoderOpen(ic.videoDecoder)
	nazalog.Assert(0, int(ret))

	err := ic.session.Pull(url, func(msg base.RtmpMsg) {
		// CHEFGREPME 2. 解封装，解码
		if msg.Header.MsgTypeId == base.RtmpTypeIdVideo {
			if msg.IsVideoKeySeqHeader() {
				var err error
				ic.spspps, err = avc.SpsPpsSeqHeader2Annexb(msg.Payload)
				nazalog.Assert(nil, err)

				return
			}

			b, err := avc.Avcc2Annexb(msg.Payload[5:])
			nazalog.Assert(nil, err)

			var p []byte
			if msg.IsVideoKeyNalu() {
				p = b
				//p = append(ic.spspps, b...)
			} else {
				p = b
			}

			x := ToCBytes(p)
			xx := (*C.uint8_t)(x)
			ret = C.LalcVideoDecoderSend(ic.videoDecoder, xx, C.int(len(p)))
			nazalog.Assert(0, int(ret))

			for {
				frame := C.av_frame_alloc()
				ret = C.LalcVideoDecoderTryReceive(ic.videoDecoder, frame)
				if ret < 0 {
					break
				}

				onVideo(frame)
			}
		}

		if msg.Header.MsgTypeId == base.RtmpTypeIdAudio  {
			if msg.IsAacSeqHeader() {
				return
			}

			p := msg.Payload[2:]
			frame := C.av_frame_alloc()
			x := ToCBytes(p)
			xx := (*C.uint8_t)(x)
			ret = C.LalcDecoderDecode(ic.audioDecoder, xx, C.int(len(p)), frame)
			nazalog.Assert(0, int(ret))

			onAudio(frame)

			C.free(x)
			//C.av_frame_unref(frame)
		}
	})
	return err
}

// ---------------------------------------------------------------------------------------------------------------------

// mixCtx
//
// CHEFGREPME 3. 合流
//
type mixCtx struct {
	m sync.Mutex
	num int
	audioQueue [][]*C.AVFrame
	videoQueue [][]*C.AVFrame
}

func (ctx *mixCtx) Init(num int) {
	ctx.num = num
	ctx.audioQueue = make([][]*C.AVFrame, num)
	ctx.videoQueue = make([][]*C.AVFrame, num)
}

func (ctx *mixCtx) FeedAudio(frame *C.AVFrame, index int) {
	ctx.m.Lock()
	defer ctx.m.Unlock()

	ctx.audioQueue[index] = append(ctx.audioQueue[index], frame)

	ctx.mixAudio()
}

func (ctx *mixCtx) FeedVideo(frame *C.AVFrame, index int) {
	ctx.m.Lock()
	defer ctx.m.Unlock()

	ctx.videoQueue[index] = append(ctx.videoQueue[index], frame)
}

func (ctx *mixCtx) mixAudio() {
	for i := 0; i < ctx.num; i++ {
		if len(ctx.audioQueue[i]) == 0 {
			return
		}
	}

	fl := C.LalcFrameListAlloc(ctx.num)
	for i := 0; i < ctx.num; i++ {
		f := ctx.audioQueue[i][0]
		ctx.audioQueue[i] = ctx.audioQueue[i][1:]
		C.LalcFrameListAdd(fl, f)
	}

	frame := C.av_frame_alloc()
	C.LalcOpAudioMixWithFrameList(fl, frame)
	C.LalcFrameListClear(fl)

	oc.FeedAudio(frame)
}

func (ctx *mixCtx) mixVideo() {
	for i := 0; i < ctx.num; i++ {
		if len(ctx.videoQueue[i]) == 0 {
			return
		}
	}

	f1 := ctx.videoQueue[0][0]
	f2 := ctx.videoQueue[1][0]
	ctx.videoQueue[0] = ctx.videoQueue[0][1:]
	ctx.videoQueue[1] = ctx.videoQueue[1][1:]

	f := C.LalcAllocAvFrameVideo(0, f1.width * 2, f2.height)
	C.LalcVideoPin(f, f1, 0, 0)
	C.LalcVideoPin(f, f2, f1.width, 0)

	oc.FeedVideo(f)
}

// ---------------------------------------------------------------------------------------------------------------------

type outCtx struct {
	debugPcmFileWriter *C.LalcPcmFileWriter

	audioEncoder *C.LalcAudioEncoder

	videoEncoder *C.LalcVideoEncoder

	pkt2rtmp *remux.AvPacket2RtmpRemuxer

	pushSession *rtmp.PushSession

	audioTs uint32
	videoTs uint32
}

func (ctx *outCtx) Init() {
	ctx.debugPcmFileWriter = C.LalcPcmFileWriterAlloc()
	C.LalcPcmFileWriterOpen(ctx.debugPcmFileWriter, (*C.char)(ToCBytes([]byte("/tmp/out.pcm"))))

	ctx.audioEncoder = C.LalcAudioEncoderAlloc()
	C.LalcAudioEncoderOpen(ctx.audioEncoder, 2, 48000)

	ctx.videoEncoder = C.LalcVideoEncoderAlloc()
	C.LalcVideoEncoderOpen(ctx.videoEncoder, 720, 1280)

	var ascCtx aac.AscContext
	ascCtx.AudioObjectType = 2
	ascCtx.SamplingFrequencyIndex = 3
	ascCtx.ChannelConfiguration = 2
	asc := ascCtx.Pack()

	ctx.pushSession = rtmp.NewPushSession()
	err := ctx.pushSession.Push("rtmp://127.0.0.1/live/test110")
	nazalog.Assert(nil, err)
	go func() {
		err := <- ctx.pushSession.WaitChan()
		nazalog.Assert(nil, err)
		os.Exit(0)
	}()

	ctx.pkt2rtmp = remux.NewAvPacket2RtmpRemuxer(func(msg base.RtmpMsg) {
		// CHEFGREPME 6. 推流
		// TODO(chef): [refactor] rtmp remuxer是否应该直接返回chunks数据，或者push session（同时考虑sub session）支持base.RtmpMsg类型的发送接口
		b := rtmp.Message2Chunks(msg.Payload, &msg.Header)
		err := ctx.pushSession.Write(b)
		nazalog.Assert(nil, err)
		//nazalog.Debugf("%+v", msg)
	})

	ctx.pkt2rtmp.InitWithAvConfig(asc, nil, nil, nil)
}

func (ctx *outCtx) FeedAudio(frame *C.AVFrame) {
	ret := C.LalcPcmFileWriterWrite(ctx.debugPcmFileWriter, frame)
	//nazalog.Debugf("%d", int(ret))

	// CHEFGREPME 4. 编码
	pkt := C.av_packet_alloc()
	ret = C.LalcAudioEncoderEncode(ctx.audioEncoder, frame, pkt)
	if ret < 0 {
		return
	}
	//C.LalcLogAvPacket(pkt)

	// CHEFGREPME 5. 封装
	ctx.pkt2rtmp.FeedAvPacket(base.AvPacket{
		PayloadType: base.AvPacketPtAac,
		Timestamp: ctx.audioTs,
		Payload: ToGoBytesRefUint8(pkt.data, pkt.size),
	})
	ctx.audioTs += 46
}

func (ctx *outCtx) FeedVideo(frame *C.AVFrame) {
	C.LalcDumpMjpeg(frame, (*C.char)(ToCBytes([]byte("/tmp/out.jpeg"))))

	ret := C.LalcVideoEncoderSend(ctx.videoEncoder, frame)
	if ret < 0 {
		return
	}
	for {
		pkt := C.av_packet_alloc()
		ret = C.LalcVideoEncoderTryReceive(ctx.videoEncoder, pkt)
		if ret < 0 {
			break
		}
		C.LalcLogAvPacket(pkt)
		pktgo := ToGoBytesRefUint8(pkt.data, pkt.size)

		avccPkt, err := Annexb2Avcc(pktgo)
		nazalog.Assert(nil, err)

		ctx.pkt2rtmp.FeedAvPacket(base.AvPacket{
			PayloadType: base.AvPacketPtAvc,
			Timestamp: ctx.videoTs,
			Payload: avccPkt,
		})
		ctx.videoTs += 66
	}
}

// ---------------------------------------------------------------------------------------------------------------------

func loop() {
	t := time.Tick(10 * time.Millisecond)
	for {
		select{
		case <- t:
		case <- ic1.session.WaitChan():
			return
		case <- ic2.session.WaitChan():
			return
		}
	}
}

func main() {
	nazalog.Init(func(option *nazalog.Option) {
		option.AssertBehavior = nazalog.AssertPanic
	})

	url1 := os.Args[1]
	url2 := os.Args[1]

	oc.Init()

	var err error

	var mixCtx mixCtx
	mixCtx.Init(2)

	err = ic1.Pull(url1,
		func(frame *C.AVFrame) {
			mixCtx.FeedAudio(frame, 0)
		},
		func(frame *C.AVFrame) {
			mixCtx.FeedVideo(frame, 0)
		})
	nazalog.Assert(nil, err)

	err = ic2.Pull(url2,
		func(frame *C.AVFrame) {
			mixCtx.FeedAudio(frame, 1)
		},
		func(frame *C.AVFrame) {
			mixCtx.FeedVideo(frame, 1)
		})
	nazalog.Assert(nil, err)

	loop()


	// pull2push
	// 演示拉一路rtmp流，再rtmp转推出去
	pushSession := rtmp.NewPushSession(func(option *rtmp.PushSessionOption) {
		option.PushTimeoutMs = 10000
		option.WriteAvTimeoutMs = 10000
	})
	pullSession := rtmp.NewPullSession(func(option *rtmp.PullSessionOption) {
		option.PullTimeoutMs = 10000
		option.ReadAvTimeoutMs = 10000
	})
	_ = pushSession.Push("rtmp://pushurl/live/xxx")
	_ = pullSession.Pull("rtmp://pullurl/live/xxx", func(msg base.RtmpMsg) {
		b := rtmp.Message2Chunks(msg.Payload, &msg.Header)
		_ = pushSession.Write(b)
	})

	select {
	case <- pushSession.WaitChan():
	case <- pullSession.WaitChan():
	}
}
