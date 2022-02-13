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
	vTrack *TrackLocalVideo
}

func (w *WebRtcSender) Init(offer webrtc.SessionDescription, udpMux ice.UDPMux, tcpMux ice.TCPMux, onICEConnectionState func(connectionState webrtc.ICEConnectionState)) (localDesc webrtc.SessionDescription, err error) {

	var peerConnection *webrtc.PeerConnection

	m := &webrtc.MediaEngine{}
	if err = m.RegisterDefaultCodecs(); err != nil {
		return
	}

	i := &interceptor.Registry{}
	if err = webrtc.RegisterDefaultInterceptors(m, i); err != nil {
		return
	}

	s := webrtc.SettingEngine{}
	s.SetICEUDPMux(udpMux)
	s.SetICETCPMux(tcpMux)

	api := webrtc.NewAPI(webrtc.WithMediaEngine(m), webrtc.WithInterceptorRegistry(i), webrtc.WithSettingEngine(s))
	peerConnection, err = api.NewPeerConnection(webrtc.Configuration{
		ICEServers: []webrtc.ICEServer{
			{
				URLs: []string{"stun:stun.l.google.com:19302"},
			},
		},
	})
	if err != nil {
		return
	}

	if w.vTrack, err = NewTrackLocalVideo(webrtc.RTPCodecCapability{MimeType: webrtc.MimeTypeH264}, "video", "lal"); err != nil {
		return
	}

	var rtpSender *webrtc.RTPSender
	if rtpSender, err = peerConnection.AddTrack(w.vTrack); err != nil {
		return
	}

	go func() {
		rtcpBuf := make([]byte, 1500)
		for {
			if _, _, err := rtpSender.Read(rtcpBuf); err != nil {
				return
			}
		}
	}()

	peerConnection.OnICEConnectionStateChange(onICEConnectionState)

	if err = peerConnection.SetRemoteDescription(offer); err != nil {
		return
	}

	var answer webrtc.SessionDescription
	if answer, err = peerConnection.CreateAnswer(nil); err != nil {
		return
	}

	if err = peerConnection.SetLocalDescription(answer); err != nil {
		return
	}

	gatherComplete := webrtc.GatheringCompletePromise(peerConnection)
	<-gatherComplete

	return *peerConnection.LocalDescription(), nil
}

func (w *WebRtcSender) Write(pkt base.AvPacket) error {
	return w.vTrack.Write(pkt)
}
