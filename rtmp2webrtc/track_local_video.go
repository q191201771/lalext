package main

import (
	"github.com/pion/rtp"
	"github.com/pion/webrtc/v3"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/naza/pkg/nazalog"
)

type TrackLocalVideo struct {
	rtpPacker *rtprtcp.RtpPacker
	rtpTrack *webrtc.TrackLocalStaticRTP
}

func NewTrackLocalVideo(c webrtc.RTPCodecCapability, id, streamID string) (*TrackLocalVideo, error) {
	pp := rtprtcp.NewRtpPackerPayloadAvc(func(option *rtprtcp.RtpPackerPayloadAvcHevcOption) {
		option.Typ = rtprtcp.RtpPackerPayloadHevcTypeAnnexb
	})
	// ssrc可以不填
	packer := rtprtcp.NewRtpPacker(pp, 90000, 0)

	rtpTrack, err := webrtc.NewTrackLocalStaticRTP(c, id, streamID)
	if err != nil {
		return nil, err
	}

	return &TrackLocalVideo{
		rtpPacker:packer,
		rtpTrack: rtpTrack,
	}, nil
}

// @param pkt: PayloadType可以不填
//
func (t *TrackLocalVideo) Write(pkt base.AvPacket) error {
	rtpPkts := t.rtpPacker.Pack(pkt)
	for i := range rtpPkts {
		var newRtpPkt rtp.Packet
		err := newRtpPkt.Unmarshal(rtpPkts[i].Raw)
		nazalog.Assert(nil, err)
		err = t.rtpTrack.WriteRTP(&newRtpPkt)
		nazalog.Assert(nil, err)
	}
	return nil
}

func (t *TrackLocalVideo) Bind(c webrtc.TrackLocalContext) (webrtc.RTPCodecParameters, error) {
	return t.rtpTrack.Bind(c)
}

func (t *TrackLocalVideo) Unbind(c webrtc.TrackLocalContext) error {
	return t.rtpTrack.Unbind(c)
}

func (t *TrackLocalVideo) ID() string {
	return t.rtpTrack.ID()
}

func (t *TrackLocalVideo) StreamID() string {
	return t.rtpTrack.StreamID()
}

func (t *TrackLocalVideo) Kind() webrtc.RTPCodecType {
	return t.rtpTrack.Kind()
}