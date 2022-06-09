// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package main

import (
	"net"

	"github.com/pion/ice/v2"
	"github.com/pion/webrtc/v3"
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/rtmp"
	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/lal/pkg/rtsp"
	"github.com/q191201771/lal/pkg/sdp"
	"github.com/q191201771/naza/pkg/nazalog"
)

type TaskCreator struct {
	udpMux ice.UDPMux
	tcpMux ice.TCPMux
	hostIp string
}

type rtspObserverAdapter struct {
	webrtcSender *WebRtcSender
	videoType    base.AvPacketPt
	sps          []byte
	pps          []byte
}

func (r *rtspObserverAdapter) OnSdp(sdpCtx sdp.LogicContext) {
	if sdpCtx.Vps != nil {
		r.videoType = base.AvPacketPtHevc
	} else {
		r.videoType = base.AvPacketPtAvc
	}
}

func (r *rtspObserverAdapter) OnRtpPacket(pkt rtprtcp.RtpPacket) {
	// do nothing
}

func (r *rtspObserverAdapter) OnAvPacket(pkt base.AvPacket) {

	switch pkt.PayloadType {
	case base.AvPacketPtAvc:
		fallthrough
	case base.AvPacketPtHevc:
		var nals [][]byte
		var err error

		if pos, len := avc.IterateNaluStartCode(pkt.Payload, 0); pos == -1 && len == -1 {
			nals, err = avc.SplitNaluAvcc(pkt.Payload)
		} else {
			nals, err = avc.SplitNaluAnnexb(pkt.Payload)
		}

		if err != nil {
			nazalog.Errorf("iterate nalu failed. err=%+v", err)
			return
		}

		for _, nal := range nals {
			var out []byte
			t := avc.ParseNaluType(nal[0])
			if t == avc.NaluTypeSps || t == avc.NaluTypePps {
				if t == avc.NaluTypeSps {
					r.sps = nal
				} else if t == avc.NaluTypePps {
					r.pps = nal
				}
			} else if t == avc.NaluTypeIdrSlice {
				out = append(out, avc.NaluStartCode3...)
				out = append(out, r.sps...)
				out = append(out, avc.NaluStartCode3...)
				out = append(out, r.pps...)
			}
			out = append(out, avc.NaluStartCode3...)
			out = append(out, nal...)

			if len(out) > 0 {
				_ = r.webrtcSender.Write(base.AvPacket{
					Timestamp: pkt.Timestamp,
					Payload:   out,
				})
			}
		}

	case base.AvPacketPtAac:
		// not support yet
	}
}

func NewTaskCreator(port int, hostIp string) (*TaskCreator, error) {
	um, err := net.ListenUDP("udp", &net.UDPAddr{
		IP:   net.IP{0, 0, 0, 0},
		Port: port,
	})

	if err != nil {
		return nil, err
	}
	tm, err := net.ListenTCP("tcp", &net.TCPAddr{
		IP:   net.IP{0, 0, 0, 0},
		Port: port,
	})
	if err != nil {
		return nil, err
	}

	return &TaskCreator{
		udpMux: webrtc.NewICEUDPMux(nil, um),
		tcpMux: webrtc.NewICETCPMux(nil, tm, 20),
		hostIp: hostIp,
	}, nil
}

func (t *TaskCreator) StartRtspTunnelTask(liveUrl string, sessionDescription string, onLocalSessDesc func(sessDesc string)) error {
	var (
		iceConnectionStateChan = make(chan webrtc.ICEConnectionState, 1)
	)

	var pullSession *rtsp.PullSession

	iceConnectionStateCB := func(connectionState webrtc.ICEConnectionState) {
		nazalog.Debugf("> OnICEConnectionStateChange. state=%s", connectionState.String())
		switch connectionState {
		case webrtc.ICEConnectionStateChecking:
			// noop
		case webrtc.ICEConnectionStateConnected:
			iceConnectionStateChan <- connectionState
		case webrtc.ICEConnectionStateFailed:
			iceConnectionStateChan <- connectionState
		case webrtc.ICEConnectionStateDisconnected:
			if pullSession != nil {
				readAlive, writeAlive := pullSession.IsAlive()
				if readAlive || writeAlive {
					pullSession.Dispose()
				}
			}
		default:
			nazalog.Errorf("NOTICEME %s", connectionState.String())
		}
	}

	var sd webrtc.SessionDescription
	err := decodeBase64Json(sessionDescription, &sd)
	if err != nil {
		return err
	}

	webrtcSender := WebRtcSender{
		UdpMux:               t.udpMux,
		TcpMux:               t.tcpMux,
		ICEConnectionStateCB: iceConnectionStateCB,
		HostIp:               t.hostIp,
	}

	observer := rtspObserverAdapter{
		webrtcSender: &webrtcSender,
	}
	pullSession = rtsp.NewPullSession(&observer, func(option *rtsp.PullSessionOption) {
		option.PullTimeoutMs = 5000
		option.OverTcp = true
	})

	localSessionDescription, err := webrtcSender.Init(sd)
	if err != nil {
		return err
	}

	encLocalSessionDescription, err := encodeBase64Json(localSessionDescription)
	if err != nil {
		return err
	}

	onLocalSessDesc(encLocalSessionDescription)

	ics := <-iceConnectionStateChan
	if ics == webrtc.ICEConnectionStateFailed {
		return ErrRtmp2WebRtc
	}

	err = pullSession.Pull(liveUrl)
	nazalog.Assert(nil, err)
	return <-pullSession.WaitChan()
}

func (t *TaskCreator) StartTunnelTask(rtmpUrl string, sessionDescription string, onLocalSessDesc func(sessDesc string)) error {
	var (
		iceConnectionStateChan = make(chan webrtc.ICEConnectionState, 1)
	)

	var sd webrtc.SessionDescription
	err := decodeBase64Json(sessionDescription, &sd)
	if err != nil {
		return err
	}

	pullSession := rtmp.NewPullSession(func(option *rtmp.PullSessionOption) {
	})

	iceConnectionStateCB := func(connectionState webrtc.ICEConnectionState) {
		nazalog.Debugf("> OnICEConnectionStateChange. state=%s", connectionState.String())
		switch connectionState {
		case webrtc.ICEConnectionStateChecking:
			// noop
		case webrtc.ICEConnectionStateConnected:
			iceConnectionStateChan <- connectionState
		case webrtc.ICEConnectionStateFailed:
			iceConnectionStateChan <- connectionState
		case webrtc.ICEConnectionStateDisconnected:
			if pullSession != nil {
				readAlive, writeAlive := pullSession.IsAlive()
				if readAlive || writeAlive {
					pullSession.Dispose()
				}
			}
		default:
			nazalog.Errorf("NOTICEME %s", connectionState.String())
		}
	}

	webrtcSender := WebRtcSender{
		UdpMux:               t.udpMux,
		TcpMux:               t.tcpMux,
		ICEConnectionStateCB: iceConnectionStateCB,
		HostIp:               t.hostIp,
	}
	localSessionDescription, err := webrtcSender.Init(sd)
	if err != nil {
		return err
	}
	encLocalSessionDescription, err := encodeBase64Json(localSessionDescription)
	if err != nil {
		return err
	}
	onLocalSessDesc(encLocalSessionDescription)

	ics := <-iceConnectionStateChan
	if ics == webrtc.ICEConnectionStateFailed {
		return ErrRtmp2WebRtc
	}

	var sps []byte
	var pps []byte
	err = pullSession.Pull(rtmpUrl, func(msg base.RtmpMsg) {
		switch msg.Header.MsgTypeId {
		case base.RtmpTypeIdMetadata:
			// noop
			return
		case base.RtmpTypeIdAudio:
			// audio not support yet
			return
		case base.RtmpTypeIdVideo:
			codecid := msg.Payload[0] & 0xF
			if codecid != base.RtmpCodecIdAvc {
				nazalog.Errorf("invalid codec id since only support avc yet. codecid=%d", codecid)
				return
			}
		}

		//nazalog.Tracef("< R len=%d, type=%s, hex=%s", len(msg.Payload), avc.ParseNaluTypeReadable(msg.Payload[5+4]), hex.Dump(nazastring.SubSliceSafety(msg.Payload, 16)))

		timestamp := msg.Header.TimestampAbs

		// 只缓存
		if msg.IsAvcKeySeqHeader() {
			sps, pps, err = avc.ParseSpsPpsFromSeqHeader(msg.Payload)
			nazalog.Assert(nil, err)
			nazalog.Debugf("cache spspps. sps=%d, pps=%d", len(sps), len(pps))
			return
		}

		var out []byte
		err = avc.IterateNaluAvcc(msg.Payload[5:], func(nal []byte) {
			t := avc.ParseNaluType(nal[0])
			//nazalog.Debugf("iterate nalu. type=%s, len=%d", avc.ParseNaluTypeReadable(nal[0]), len(nal))
			if t == avc.NaluTypeSei {
				return
			}

			if t == avc.NaluTypeIdrSlice {
				out = append(out, avc.NaluStartCode3...)
				out = append(out, sps...)
				out = append(out, avc.NaluStartCode3...)
				out = append(out, pps...)
				nazalog.Debugf("append spspps. sps=%d, pps=%d", len(sps), len(pps))
			}

			out = append(out, avc.NaluStartCode3...)
			out = append(out, nal...)
		})

		if len(out) != 0 {
			_ = webrtcSender.Write(base.AvPacket{
				Timestamp: timestamp,
				Payload:   out,
			})
		}
	})
	if err != nil {
		return err
	}
	return <-pullSession.WaitChan()
}
