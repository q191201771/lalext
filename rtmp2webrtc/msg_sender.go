package main

import (
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/nazalog"
)

type MsgSender struct {
	webrtcSender *WebRtcSender
	audioProcess *AudioProcess
	sps          []byte
	pps          []byte
}

func (r *MsgSender) ProcessRtmpMsg(msg base.RtmpMsg) {

	switch msg.Header.MsgTypeId {
	case base.RtmpTypeIdMetadata:
		// noop
		return
	case base.RtmpTypeIdAudio:
		tmpMsg := msg.Clone()
		r.audioProcess.AddMsg(tmpMsg)
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

	var err error
	// 只缓存
	if msg.IsAvcKeySeqHeader() {
		r.sps, r.pps, err = avc.ParseSpsPpsFromSeqHeader(msg.Payload)
		nazalog.Assert(nil, err)
		nazalog.Debugf("cache spspps. sps=%d, pps=%d", len(r.sps), len(r.pps))
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
			out = append(out, r.sps...)
			out = append(out, avc.NaluStartCode3...)
			out = append(out, r.pps...)
			nazalog.Debugf("append spspps. sps=%d, pps=%d", len(r.sps), len(r.pps))
		}

		out = append(out, avc.NaluStartCode3...)
		out = append(out, nal...)
	})

	if len(out) != 0 {
		_ = r.webrtcSender.WriteVideo(base.AvPacket{
			Timestamp: int64(timestamp),
			Payload:   out,
		})
	}
}
