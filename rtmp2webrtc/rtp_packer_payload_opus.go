package main

type RtpPackerPayloadOpus struct {
}

func NewRtpPackerPayloadOpus() *RtpPackerPayloadOpus {
	return &RtpPackerPayloadOpus{}
}

func (r *RtpPackerPayloadOpus) Pack(in []byte, maxSize int) (out [][]byte) {

	out = append(out, in)
	return
}
