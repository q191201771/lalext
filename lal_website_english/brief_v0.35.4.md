# LAL v0.35.4 released, OBS supports RTMP H.265 push streaming, I'm with you

Go language streaming open source project [LAL](https://github.com/q191201771/lal) released v0.35.4 today.

> LAL project address: https://github.com/q191201771/lal

The old rules — a brief introduction:

#### ▦ I. OBS supports RTMP H.265 push streaming

The new standard, generally known as _enhanced RTMP_, has already been implemented in the new version of OBS (version 29.1+ [click to download the OBS installer](https://github.com/obsproject/obs-studio/releases)). To be able to use it, LAL has also made the corresponding adaptation; in other words, now you can use OBS to push H.265 RTMP streams to LAL.

**Tip:** The advantage of using H.265 is that compared to H.264, it requires a lower bitrate but has a higher image quality.

A few details worth noting:

1. After LAL converts a stream to a different encapsulation protocol (e.g. RTSP, HLS), the output protocol is still the standard protocol, which is not different from the original one.
2. Even when the LAL output is set to RTMP/FLV, it nevertheless keeps the enhanced RTMP format. Currently, the only player adapted to the enhanced RTMP is `mpegts.js`, but subsequent `ffmpeg` and VLC versions (as well as other players) will soon support it as well.
3. Enhanced RTMP and the previous Jinshan Cloud protocol to enable HEVC over RTMP (that is, CodecID = 12, supported by most of the Chinese domestic CDN providers) are two different things. LAL supports both; the current strategy is to use the RTMP/FLV protocol for input and output, but subsequent versions may require conversion (e.g. through the configuration file or by having URL parameters, etc.).

For details on the implementation of the enhanced RTMP protocol, and how to use its OBS counterpart, you can take a look at our article: [《enhanced RTMP》](enhanced_rtmp.md)

#### ▦ II. G711A/G711U

The last version already supported G.711 audio encoding format (A-law and µ-law), but it left a feature to be implemented: RTSP to RTMP with video only, without sound. This version supports that as well.

We're in the process of finishing up the overall G.711 implementation: https://www.yuque.com/pengrl/public/psxbp37r3yqopnxx 🇨🇳

#### ▦ III. RTSP TCP/UDP switching

An optimisation: some RTSP sources don't support TCP to transfer audio/video data, they will reply with status code 461 to the other end in the SETUP signaling phase to indicate that they don't support TCP. The new version of LAL will try to switch to UDP SETUP when it receives the 461 status code, and vice versa for UDP to TCP.

#### ▦ More!

There are a few more modifications, not covered individually, and roughly as follows:

> - [opt] mpegts packs patpmt according to encoding format (improves compatibility with pure video streaming)
> - [opt] HTTP-API: cross-domain support
> - [fix] rtmp: parse amf strict array
> - [fix] rtmp to mpegts handling timestamps incorrectly, causing ffplay to get an error when playing hls with b-frames
> - [fix] GetSamplingFrequency missing 24000
> - [fix] simplifing logic to enable `fragment_duration_ms` on configuration to be under thousands (but not below hundreds)
> - [fix] fix a crash when CustomizePubSessionContext uses dumpFile null pointer
> - [test] unit test for base.DumpFile
>
> The above is extracted from the [lal changelog](../CHANGELOG.md), you can get more details from the source document.

#### ▦ Developers

Thanks to the open source contributors involved in this release: yoko, ZSC714725, joaop, sanenchen, yang heng01~, penglh, LiH0820

#### ▦ Learn more about lal

- [GitHub](https://github.com/q191201771/lal)
- [Official Documentation](https://pengrl.com/lal)
- [Contact Author](https://pengrl.com/lal/#/Author)

WeChat scan QR code to add yourself as the author's friend (into the WeChat group):

![WeChat](https://pengrl.com/images/yoko_vx.jpeg?date=2304)

End of this article. Have a nice day!

yoko, 202304
