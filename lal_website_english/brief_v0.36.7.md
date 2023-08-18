# LAL v0.36.7 released, Customize Sub, I have all for you!

Go language streaming open source project [LAL](https://github.com/q191201771/lal) released v0.36.7 today.

> LAL project address: https://github.com/q191201771/lal

The old rules — a brief introduction:

▦ Customize Sub, I have all for you!

This is an important feature for those who use lalserver for secondary development. The business side can get the streams inside lalserver by setting the callback function.

What is the use of getting the streams? Take lal's derivative project **lalmax** for example, it is based on lalserver's Customize Sub, which extends the ability of SRT and WebRTC playback for all the streams inputted into lalserver.

Here is also a brief introduction to **lalmax**, which will be an important project in the overall lal ecosystem. It is based on the secondary development interface provided by lalserver, and combines it with third-party libraries to support more protocols and provide richer functionality.  
Accordingly, lal project will continue to maintain its own no third-party dependencies, high performance and other features.  
The lalmax project is located at https://github.com/q191201771/lalmax

If you want to understand the Customize Sub better, you can read the following two documents related to Customize secondary development first:

- [2.6 lalserver secondary development (plug-in)](https://pengrl.com/#/customize)
- [|-- 2.6.1 Secondary Development - pub access to custom streams](https://pengrl.com/#/customize_pub)

If you want to see the code directly:

- The corresponding API signature in lal: https://github.com/q191201771/lal/blob/master/pkg/logic/logic.go#L43
- Example of lalmax usage: https://github.com/q191201771/lalmax/blob/master/main.go#L39

▦ RTMP compatibility optimisation

- There are two formats in the AMF encoding that don't contain valid content and are not really useful, called Undefined and Unsupported, which were not parsed by lal before, but are actually used by some clients in reality.
- Compatible with publish signalling without a pubType field. Yes, some clients are capricious, but since they are used by many people, they will angrily complain when the streaming doesn't work as expected..
- Handles the case where the audio format is written in the metadata instead of in a separate seq header packet.
- `Rtmp2AvPacketRemuxer` — add parameter to specify whether SEI needs to be discarded or not.

▦ RTSP optimisation

- Support for handling timestamp rollover cases. That is, the timestamp is incremented to a very large number and then reverts back to a smaller number to continue incrementing.
- Package RTSP: A new configuration variable `BaseInSessionTimestampFilterFlag` has been added to determine whether the timestamps of RTSP-synthesised frame-level packets use the timestamps in RTSP, or are reset to start at 0.

▦ lalserver optimisation

- Asynchronise NotifyHandler callbacks into a separate goroutine. The purpose is to make it easier for the business side to freely call other API interfaces of lalserver via callbacks.
- Add fps field to the streaming information available from the HTTP API.
- Disable auto stop relay pull function in case of business side `WithOnHookSession`

▦ Bug fixes

Firstly, there is a serious bug that causes HLS to be unplayable, which was introduced in the last version v0.35.4: when converting TS files, the AVC flag in PMT was wrongly written as AAC due to a typo.

Furthermore, the following bugs in protocol handling were addressed:

- rtp: fix bug in parsing ext extension data
- remux: `Rtmp2AvPacketRemuxer` multi-slice append sps error caused splash screen.
- rtmp2mpegts: make sure pts has a value
- rtmp: Asynchronous sending is not necessary when `WriteChanSize` is 0 in `ClientSession` configuration.

Bug with releasing resources in lalserver:

- When playing non-existent RTSP streams, timeout does not release completely.

Some other minor bugs:

- Check if nil before using callback object `IMuxerObserver` in HLS

▦ More

There are some other changes not presented individually, roughly as follows:

> - [chore] all shell file go to script folder
> - [chore] all scripts +x to add execute permissions to make sure CI works properly.
> - [chore] fix macOS readlink no -f parameter causes script execution failure problem
> - [fix] WebUI: read null when no group
> - [fix] connection: not working set ModWriteChanSize
> - [fix] not working timout for RTMP server session
> - [test] dump rtsp test support video
> - [refactor] Fixing all cross-domain code
> - [refactor] fix all timeout related code
> - [refactor] HEVC: expose all fields in hevc.Context
> - [refactor] AVC: Expose sps struct fields in avc.Context
> Avc: Exposes the Sps struct field in avc.Context.
> The above is extracted from the [lal changelog](https://pengrl.com/lal/#/CHANGELOG), you can get more details from the source document.

▦ Developers

Thanks to the open source contributors who worked on this release: yoko(Liang), ZSC714725(Ah Loud), HustCoderHu(Xiao Hu), Jae-Sung Lee(Ah Han)

▦ Learn more about lal

- [GitHub](https://github.com/q191201771/lal)
- [Official Documentation](https://pengrl.com/lal)
- [Contact Author](https://pengrl.com/lal/#/Author)

WeChat scan code to add yourself as a friend (into the WeChat group):

![WeChat](https://pengrl.com/images/yoko_vx.jpeg?date=2304)

End of this article. Have a nice day!

yoko, 202307
