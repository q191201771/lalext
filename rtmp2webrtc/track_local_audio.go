// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package main

import (
	"github.com/pion/rtp"
	"github.com/pion/webrtc/v3"
	"github.com/pion/webrtc/v3/pkg/media/oggwriter"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/naza/pkg/nazalog"
)

type TrackLocalAudio struct {
	rtpPacker *rtprtcp.RtpPacker
	rtpTrack  *webrtc.TrackLocalStaticRTP
	ogg       *oggwriter.OggWriter
	count     int
}

func NewTrackLocalAudio(c webrtc.RTPCodecCapability, id, streamID string) (*TrackLocalAudio, error) {
	pp := NewRtpPackerPayloadOpus()

	// ssrc可以不填
	packer := rtprtcp.NewRtpPacker(pp, 48000, 0)

	rtpTrack, err := webrtc.NewTrackLocalStaticRTP(c, id, streamID)
	if err != nil {
		return nil, err
	}

	// ogg, _ := oggwriter.New("output.ogg", 48000, 2)
	return &TrackLocalAudio{
		rtpPacker: packer,
		rtpTrack:  rtpTrack,
		// ogg:       ogg,
	}, nil
}

// @param pkt: PayloadType可以不填
//
func (t *TrackLocalAudio) Write(pkt base.AvPacket) error {
	rtpPkts := t.rtpPacker.Pack(pkt)
	for i := range rtpPkts {
		var newRtpPkt rtp.Packet
		err := newRtpPkt.Unmarshal(rtpPkts[i].Raw)
		nazalog.Assert(nil, err)
		err = t.rtpTrack.WriteRTP(&newRtpPkt)
		nazalog.Assert(nil, err)

		// for debug
		// if t.count > 1000 {
		// 	break
		// } else {
		// 	t.ogg.WriteRTP(&newRtpPkt)
		// 	t.count++
		// 	if t.count > 1000 {
		// 		nazalog.Info("close ogg file..............")
		// 	}
		// }

	}
	return nil
}

func (t *TrackLocalAudio) Bind(c webrtc.TrackLocalContext) (webrtc.RTPCodecParameters, error) {
	return t.rtpTrack.Bind(c)
}

func (t *TrackLocalAudio) Unbind(c webrtc.TrackLocalContext) error {
	return t.rtpTrack.Unbind(c)
}

func (t *TrackLocalAudio) ID() string {
	return t.rtpTrack.ID()
}

func (t *TrackLocalAudio) StreamID() string {
	return t.rtpTrack.StreamID()
}

func (t *TrackLocalAudio) Kind() webrtc.RTPCodecType {
	return t.rtpTrack.Kind()
}

func (t *TrackLocalAudio) RID() string {
	return t.rtpTrack.RID()
}
