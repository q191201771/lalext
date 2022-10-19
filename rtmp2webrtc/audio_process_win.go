//go:build windows
// +build windows

package main

type AudioProcess struct{}

func NewAudioProcess(sender *WebRtcSender) *AudioProcess {
	return &AudioProcess{}
}

func (r *AudioProcess) AddMsg(msg base.RtmpMsg) {

}

func (r *AudioProcess) Quit() {

}

func (r *AudioProcess) ProcessAudio(msg base.RtmpMsg) {}
