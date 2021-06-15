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
	"github.com/pion/webrtc/v3"
	"time"

	"github.com/pion/webrtc/v3/pkg/media"
)

var (
	ErrRtmp2WebRtc = errors.New("rtmp2webrtc: fxxk")
)

type WebRtcSender struct {
	videoTrack *webrtc.TrackLocalStaticSample
}

func (w *WebRtcSender) Init(offer webrtc.SessionDescription, onICEConnectionState func(connectionState webrtc.ICEConnectionState)) (localDesc webrtc.SessionDescription, err error) {
	var peerConnection *webrtc.PeerConnection
	peerConnection, err = webrtc.NewPeerConnection(webrtc.Configuration{
		ICEServers: []webrtc.ICEServer{
			{
				URLs: []string{"stun:stun.l.google.com:19302"},
			},
		},
	})
	if err != nil {
		return
	}

	if w.videoTrack, err = webrtc.NewTrackLocalStaticSample(webrtc.RTPCodecCapability{MimeType: webrtc.MimeTypeH264}, "video", "lal"); err != nil {
		return
	}

	var rtpSender *webrtc.RTPSender
	if rtpSender, err = peerConnection.AddTrack(w.videoTrack); err != nil {
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

	return *peerConnection.LocalDescription(), nil
}

func (w *WebRtcSender) Write(nal []byte, timestamp uint32) error {
	//nazalog.Tracef("> W type=%s, len=%d", avc.ParseNALUTypeReadable(nal[0]), len(nal))
	return w.videoTrack.WriteSample(media.Sample{Data: nal, Duration: time.Duration(timestamp) * time.Millisecond})
}
