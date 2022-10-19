//go:build !windows
// +build !windows

package main

/*
#cgo LDFLAGS: -lavcodec -lavformat -lswresample -lavutil
#include "audio_process.c"
static int init_aac_2_opus_contex(struct AudioPrcessContex *contex);
static void clean_up_audio_process_contex(struct AudioPrcessContex *ctx);
static int fifo_enough_to_encode(AudioPrcessContex *ctx);
static int encode_and_read(int8_t *out_data, int *out_len, int *encoded_samples, AudioPrcessContex *ctx);
static int decode_resample_and_store(int8_t *in_data, int in_len, AudioPrcessContex *ctx);
*/
import "C"

import (
	"os"
	"unsafe"

	"github.com/q191201771/lal/pkg/aac"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/nazalog"
)

const (
	kPerfectSamplerate = 48000
	kPerfectChannels   = 2
)

type AudioProcess struct {
	// webrtc param
	webrtcSender *WebRtcSender
	quit         chan bool
	msgQueue     chan base.RtmpMsg
	timestamp    int64

	// codec param
	initFlag bool

	//  cgo process
	audioContex *C.AudioPrcessContex

	dump *os.File
}

func NewAudioProcess(sender *WebRtcSender) *AudioProcess {

	process := AudioProcess{
		webrtcSender: sender,
		quit:         make(chan bool),
		msgQueue:     make(chan base.RtmpMsg, 100),
	}

	// for dump
	// process.dump, _ = os.Create("go_raw_data.bin")

	go func() {
		for {
			select {
			case msg := <-process.msgQueue:
				process.ProcessAudio(msg)
			case <-process.quit: // quit
				return
			}
		}
	}()
	return &process
}

func (r *AudioProcess) AddMsg(msg base.RtmpMsg) {
	r.msgQueue <- msg
}

func (r *AudioProcess) Quit() {
	//clean up audio process
	C.clean_up_audio_process_contex(r.audioContex)

	r.quit <- true
}

func (r *AudioProcess) ProcessAudio(msg base.RtmpMsg) {

	if msg.Header.MsgTypeId != base.RtmpTypeIdAudio {
		return
	}

	if msg.IsAacSeqHeader() { // init audio process
		asc := msg.Payload[2:]
		ascCtx, err := aac.NewAscContext(asc)
		if err == nil {
			frequence, _ := ascCtx.GetSamplingFrequency()
			context := &C.AudioPrcessContex{
				in_samplerate: C.int(frequence),
				in_channels:   C.int(ascCtx.ChannelConfiguration),
				// FIXME: 怎么选择合适的 samplefmt,ffmpeg里面输出的为fltp
				in_format: 1, // fmt: s16le, reference:https://ffmpeg.org/doxygen/4.4/group__lavu__sampfmts.html#gaf9a51ca15301871723577c730b5865c5

				out_samplerate: kPerfectSamplerate,
				out_channels:   kPerfectChannels,
				out_format:     1, // fmt s16le
				enable_dump:    0, // 会在本地生成一个ogg文件,用于调试
			}

			ret := C.init_aac_2_opus_contex(context)
			if ret != 0 {
				nazalog.Info("faile to init aac 2 opus context")
				return
			}
			r.audioContex = context
			r.initFlag = true
		}
		return
	}

	if !r.initFlag {
		return
	}

	aac := msg.Payload[2:]

	if r.timestamp == 0 {
		r.timestamp = int64(msg.Header.TimestampAbs)
		nazalog.Info("init time stamp:", r.timestamp)
	}

	ret := C.decode_resample_and_store((*C.int8_t)(unsafe.Pointer(&aac[0])), C.int(len(aac)), r.audioContex)
	if ret != 0 {
		nazalog.Info("decode_resample_and_store failed: ", ret)
		return
	}

	for {
		enough := C.fifo_enough_to_encode(r.audioContex)
		if enough == 0 {
			break
		}

		converedBuffer := make([]byte, 1024)
		var convertedLen int
		var encodedSamples int

		ret := C.encode_and_read((*C.int8_t)(unsafe.Pointer(&converedBuffer[0])), (*C.int)(unsafe.Pointer(&convertedLen)), (*C.int)(unsafe.Pointer(&encodedSamples)), r.audioContex)
		if ret != 0 {
			nazalog.Info("encode_and_read failed:", ret)
			return
		}

		deltaTimeStamp := int64(1000.0 / (kPerfectSamplerate / encodedSamples))

		r.timestamp += deltaTimeStamp
		if err := r.webrtcSender.WriteAudio(base.AvPacket{
			Timestamp: int64(r.timestamp),
			Payload:   converedBuffer[:convertedLen],
		}); err != nil {
			nazalog.Info("audio package write failed:", err)
		}
	}

	return

}

func Stereo2Mono(data []byte, bytesPersample int) []byte {
	originSize := len(data)
	mono := make([]byte, originSize/2)

	var inc int
	for i := 0; i < originSize; i++ {
		if i%bytesPersample < bytesPersample/2 {
			mono[inc] = data[i]
			inc++
		}
	}
	return mono
}
