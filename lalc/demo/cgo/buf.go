package main

// #include <stdint.h>
//
import "C"
import (
	"reflect"
	"unsafe"
)


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
