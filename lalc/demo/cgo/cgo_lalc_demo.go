package main

//#cgo CFLAGS: -I../../liblalc
//#cgo LDFLAGS: -L${SRCDIR}/../../output -llalc -lavfilter -lavformat -lavdevice -lavcodec -lavutil -lpostproc -lswresample -lswscale -lx264 -lx265 -lfdk-aac -lopus -lm -lz -ldl
//
//#include "lalc.h"
//
import "C"
import (
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/rtmp"
	"github.com/q191201771/naza/pkg/nazalog"
	"os"
	"reflect"
	"sync"
	"time"
	"unsafe"
)

var (
	ic1 inCtx
	ic2 inCtx
)

type inCtx struct {
	session *rtmp.PullSession
	decoder *C.LalcDecoder

    videoDecoder *C.LalcVideoDecoder

	m sync.Mutex
	queue []*C.AVFrame
}

//type onFrame func(frame *C.AVFrame)

func (ic *inCtx) Pull(url string) error {
	ic.session = rtmp.NewPullSession()

	ic.decoder = C.LalcDecoderAlloc()
	ret := C.LalcDecoderOpen(ic.decoder, 2, 48000)
	nazalog.Assert(0, int(ret))

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
			ret = C.LalcDecoderDecode(ic.decoder, xx, C.int(len(p)), frame)
			nazalog.Assert(0, int(ret))

			ic.Push(frame)

			C.free(x)
			C.av_frame_unref(frame)
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

func loop() {
	t := time.Tick(10 * time.Millisecond)
	for {
		select{
		case <- t:
			if !ic1.Empty() && !ic2.Empty() {
				f1 := ic1.Pop()
				f2 := ic2.Pop()

				frame := C.av_frame_alloc()

				fl := C.LalcFrameListAlloc(2)
				C.LalcFrameListAdd(fl, f1)
				C.LalcFrameListAdd(fl, f2)
				C.LalcOpAudioMixWithFrameList(fl, frame)
			}
		case <- ic1.session.WaitChan():
			return
		case <- ic2.session.WaitChan():
			return
		}
	}
}

func main() {
	url1 := os.Args[1]
	url2 := os.Args[2]

	var err error

	err = ic1.Pull(url1)
	nazalog.Assert(nil, err)

	err = ic2.Pull(url2)
	nazalog.Assert(nil, err)

	loop()
}

// TODO(chef): [refactor] to nazaext

// ---------------------------------------------------------------------------------------------------------------------

// ToGoBytesChar 将C内存块转换为Go字符切片，注意，内部发生内存拷贝
//
func ToGoBytesChar(data *C.char, length C.int) []byte {
	return C.GoBytes(unsafe.Pointer(data), length)
}

// ToGoBytesRefUint8 将C内存块转换为Go字符切片，注意，内部不发生内存拷贝
//
func ToGoBytesRefUint8(data *C.uint8_t, length C.int) []byte {
	var ret []byte
	header := (*reflect.SliceHeader)(unsafe.Pointer(&ret))
	header.Data = uintptr(unsafe.Pointer(data))
	header.Len = int(length)
	header.Cap = int(length)
	return ret
}

// ToGoBytesRefChar 和 ToGoBytesRefUint8 功能相同，区别是输入参数 data 从C的 `uint8_t *` 变更为 `char *`
//
func ToGoBytesRefChar(data *C.char, length C.int) []byte {
	var ret []byte
	header := (*reflect.SliceHeader)(unsafe.Pointer(&ret))
	header.Data = uintptr(unsafe.Pointer(data))
	header.Len = int(length)
	header.Cap = int(length)
	return ret
}

// ---------------------------------------------------------------------------------------------------------------------

// ToCBytes Go字符切片转换为C内存块，注意，内部发生内存拷贝
//
// 之后可以这样转换为需要的C指针类型(以下假设GoToCBytes的返回值用`ret`变量接收):
//     (*C.char)ret
//     (*C.uint8_t)ret
//
// 注意，使用结束后，需要调用以下函数释放C内存:
//     C.free(ret)
//
func ToCBytes(b []byte) unsafe.Pointer {
	return C.CBytes(b)
}

// ToCBytesRef Go字符切片转换为C内存块，注意，内部不发生内存拷贝
//
// 之后可以这样转换为需要的C指针类型(以下假设GoToCBytes的返回值用`ret`变量接收):
//     (*C.char)ret
//     (*C.uint8_t)ret
//
func ToCBytesRef(b []byte) unsafe.Pointer {
	p := (*reflect.SliceHeader)(unsafe.Pointer(&b))
	return unsafe.Pointer(p.Data)
}

// ---------------------------------------------------------------------------------------------------------------------
