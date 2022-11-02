// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package main

import (
	"errors"

	"github.com/pion/ice/v2"
	"github.com/pion/interceptor"
	"github.com/pion/webrtc/v3"
	"github.com/q191201771/lal/pkg/base"
)

var (
	ErrRtmp2WebRtc = errors.New("rtmp2webrtc: fxxk")
)

type WebRtcSender struct {
	vTrack               *TrackLocalVideo
	ATrack               *TrackLocalAudio
	UdpMux               ice.UDPMux
	TcpMux               ice.TCPMux
	HostIp               string
	ICEConnectionStateCB func(connectionState webrtc.ICEConnectionState)
	peerConnection       *webrtc.PeerConnection
}

func (w *WebRtcSender) Init(offer webrtc.SessionDescription) (localDesc webrtc.SessionDescription, err error) {

	m := &webrtc.MediaEngine{}
	if err = m.RegisterDefaultCodecs(); err != nil {
		return
	}

	i := &interceptor.Registry{}
	if err = webrtc.RegisterDefaultInterceptors(m, i); err != nil {
		return
	}

	s := webrtc.SettingEngine{
		LoggerFactory: rtcLoggerFactory{},
	}
	s.SetICEUDPMux(w.UdpMux)
	s.SetICETCPMux(w.TcpMux)

	webrtcConf := webrtc.Configuration{}
	if w.HostIp != "" {
		s.SetLite(true)
		s.SetNAT1To1IPs([]string{w.HostIp}, webrtc.ICECandidateTypeHost)
	} else {
		webrtcConf.ICEServers = []webrtc.ICEServer{
			{
				URLs: []string{"stun:stun2.seewo.com:3478"},
			},
		}
	}

	api := webrtc.NewAPI(webrtc.WithMediaEngine(m), webrtc.WithInterceptorRegistry(i), webrtc.WithSettingEngine(s))
	w.peerConnection, err = api.NewPeerConnection(webrtcConf)
	if err != nil {
		return
	}

	if w.vTrack, err = NewTrackLocalVideo(webrtc.RTPCodecCapability{MimeType: webrtc.MimeTypeH264}, "video", "lal"); err != nil {
		return
	}

	if w.ATrack, err = NewTrackLocalAudio(webrtc.RTPCodecCapability{MimeType: "audio/opus"}, "audio", "pion"); err != nil {
		return
	}

	var videoRtpSender *webrtc.RTPSender
	if videoRtpSender, err = w.peerConnection.AddTrack(w.vTrack); err != nil {
		return
	}

	var audioRtpSender *webrtc.RTPSender
	if audioRtpSender, err = w.peerConnection.AddTrack(w.ATrack); err != nil {
		return
	}

	go func() {
		rtcpBuf := make([]byte, 1500)
		for {
			if _, _, err := audioRtpSender.Read(rtcpBuf); err != nil {
				return
			}
		}
	}()

	go func() {
		rtcpBuf := make([]byte, 1500)
		for {
			if _, _, err := videoRtpSender.Read(rtcpBuf); err != nil {
				return
			}
		}
	}()

	w.peerConnection.OnICEConnectionStateChange(w.ICEConnectionStateCB)

	if err = w.peerConnection.SetRemoteDescription(offer); err != nil {
		return
	}

	var answer webrtc.SessionDescription
	if answer, err = w.peerConnection.CreateAnswer(nil); err != nil {
		return
	}

	if err = w.peerConnection.SetLocalDescription(answer); err != nil {
		return
	}

	gatherComplete := webrtc.GatheringCompletePromise(w.peerConnection)
	<-gatherComplete

	return *w.peerConnection.LocalDescription(), nil
}

func (w *WebRtcSender) WriteVideo(pkt base.AvPacket) error {
	return w.vTrack.Write(pkt)
}

func (w *WebRtcSender) WriteAudio(pkt base.AvPacket) error {
	return w.ATrack.Write(pkt)
}

func (w *WebRtcSender) Close() {
	w.peerConnection.Close()
}
