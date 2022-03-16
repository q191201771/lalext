package main

//#cgo CFLAGS: -I../../liblalc
//#cgo LDFLAGS: -L${SRCDIR}/../../output -llalc -lavfilter -lavformat -lavdevice -lavcodec -lavutil -lpostproc -lswresample -lswscale -lx264 -lx265 -lfdk-aac -lopus -lm -lz -ldl
//
//#include "lalc.h"
//
import "C"
import (
	"github.com/q191201771/lal/pkg/aac"
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

type inCtx struct {
	session *rtmp.PullSession

	audioDecoder *C.LalcDecoder

    videoDecoder *C.LalcVideoDecoder

	m sync.Mutex
	queue []*C.AVFrame
}


func (ic *inCtx) Pull(url string) error {
	ic.session = rtmp.NewPullSession()

	ic.audioDecoder = C.LalcDecoderAlloc()
	ret := C.LalcDecoderOpen(ic.audioDecoder, 2, 48000)
	nazalog.Assert(0, int(ret))

	ic.videoDecoder = C.LalcVideoDecoderAlloc()
    ret = C.LalcVideoDecoderOpen(ic.videoDecoder)
	nazalog.Assert(0, int(ret))

	err := ic.session.Pull(url, func(msg base.RtmpMsg) {
		if msg.Header.MsgTypeId == base.RtmpTypeIdVideo {
		}

		if msg.Header.MsgTypeId == base.RtmpTypeIdAudio  {
			if msg.Payload[1] == base.RtmpAacPacketTypeSeqHeader {
				return
			}

			p := msg.Payload[2:]
			frame := C.av_frame_alloc()
			x := ToCBytes(p)
			xx := (*C.uint8_t)(x)
			ret = C.LalcDecoderDecode(ic.audioDecoder, xx, C.int(len(p)), frame)
			nazalog.Assert(0, int(ret))

			ic.Push(frame)

			C.free(x)
			//C.av_frame_unref(frame)
		}
	})
	return err
}

func (ic *inCtx) Empty() bool {
	return len(ic.queue) == 0
}

func (ic *inCtx) Push(f *C.AVFrame) {
	ic.queue = append(ic.queue, f)
}

func (ic *inCtx) Pop() *C.AVFrame {
	f := ic.queue[0]
	ic.queue = ic.queue[1:]
	return f
}

// ---------------------------------------------------------------------------------------------------------------------

type outCtx struct {
	debugPcmFileWriter *C.LalcPcmFileWriter

	audioEncoder *C.LalcAudioEncoder

	pkt2rtmp *remux.AvPacket2RtmpRemuxer

	pushSession *rtmp.PushSession

	ts uint32
}

func (ctx *outCtx) Init() {
	ctx.debugPcmFileWriter = C.LalcPcmFileWriterAlloc()
	C.LalcPcmFileWriterOpen(ctx.debugPcmFileWriter, (*C.char)(ToCBytes([]byte("/tmp/out.pcm"))))

	ctx.audioEncoder = C.LalcAudioEncoderAlloc()
	C.LalcAudioEncoderOpen(ctx.audioEncoder, 2, 48000)

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
	nazalog.Debugf("%d", int(ret))

	pkt := C.av_packet_alloc()
	ret = C.LalcAudioEncoderEncode(ctx.audioEncoder, frame, pkt)
	if ret < 0 {
		return
	}
	C.LalcLogAvPacket(pkt)
	ctx.pkt2rtmp.FeedAvPacket(base.AvPacket{
		PayloadType: base.AvPacketPtAac,
		Timestamp: ctx.ts,
		Payload: ToGoBytesRefUint8(pkt.data, pkt.size),
	})
	ctx.ts += 46
}

// ---------------------------------------------------------------------------------------------------------------------

func loop() {
	t := time.Tick(10 * time.Millisecond)
	for {
		select{
		case <- t:
			if !ic1.Empty() && !ic2.Empty() {
				f1 := ic1.Pop()
				f2 := ic2.Pop()

				C.LalcLogAvFrame(f1)
				frame := C.av_frame_alloc()

				fl := C.LalcFrameListAlloc(2)
				C.LalcFrameListAdd(fl, f1)
				C.LalcFrameListAdd(fl, f2)
				C.LalcOpAudioMixWithFrameList(fl, frame)
				C.LalcFrameListClear(fl)
				oc.FeedAudio(f1)
			}
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

	err = ic1.Pull(url1)
	nazalog.Assert(nil, err)

	err = ic2.Pull(url2)
	nazalog.Assert(nil, err)

	loop()
}
