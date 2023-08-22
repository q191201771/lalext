# LAL v0.33.0 released to support capture streaming data playback debugging

Go language streaming media open source project [LAL](https://github.com/q191201771/lal) today released v0.33.0 version.

> LAL project address: https://github.com/q191201771/lal

The old rules: first pick three changes, briefly explained:

#### ▌I, Support capture stream data playback debugging

This feature can greatly improve the efficiency of LAL users to give feedback on problems and identify then, thus improving the compatibility of LAL.

For details, I wrote a separate document: ["debug dump capture lal stream debugging"](https://pengrl.com/lal/#/debug_dump)

#### ▌II, HTTP-API and HTTP-Notify callbacks provide richer information about HLS protocols

As we all know, HLS is played by the client continuously initiating HTTP request after HTTP request, fetching one TS stream fragment file at a time.  
This short-connection method leads to the server not being able to distinguish between different players, and not being able to judge the point in time when a player starts playing and ends playing.

To address this issue, we support it by adding 302 jumps, adding a parameter with user unique ID to the `m3u8` URL, and adding that ID to multiple TSs of a user:

- Getting the number of HLS players, bitrate, etc.
- Get event notification when HLS starts or ends playing.

There are two configurations for this feature in the config file, see [hls/sub_session_timeout_ms](ConfigBrief.md#-hlssub_session_timeout_ms) and [hls/sub_session_hash_key](ConfigBrief.md#-hlssub_session_hash_key)

If you don't like the overhead of `m3u8` 302 jumps, and you don't care about HLS stats and notifications, then you can turn that off via the config file.

> Related documentation:  
> [The lalserver HTTP API Interface](HTTPAPI.md)  
> [The lalserver HTTP Notify (Callback/Webhook) event callback](HTTPNotify.md)

#### ▌III. Parsing RTP extension headers

Added logic to parse RTP extension headers, allowing better support for RTSP streams.

#### ▌More modifications

There are some other modifications that are not explained one by one, roughly as follows:

> - [chore] docker supports both amd and arm architectures
> - [feat] demo: analyseflv supports http flv streams or flv files as input.
> - [feat] plugin example: add example to read flv file and input data to lalserver via CustomPubSession.
> - [opt] rtmp: reduces the size of memory preallocated for chunking.
> - [opt] Plugin: Customise Pub supports AvPacket and RtmpMsg.
> - [opt] Gop Buffering supports configuring the maximum number of buffered frames in a single Gop.
> - [fix] Fix the problem that HLS fails to get app name.
> - [fix] flv: fix the bug where ReadAllTagsFromFlvFile does not close the file
> - [fix] rtmp: valid length check before receiving buff parsing
> - [fix] rtmp.
>
> The above is extracted from ["lal CHANGELOG Release Log"](../CHANGELOG), you can get more details from the source document.

#### ▦ Learn more about lal

- [GitHub](https://github.com/q191201771/lal)
- [Official Documentation](https://pengrl.com/lal)
- [Contact Author](https://pengrl.com/lal/#/Author)

WeChat scan QR code to add yourself as the author's friend (into the WeChat group):

![WeChat](https://pengrl.com/images/yoko_vx.jpeg?date=2304)

End of this article. Have a nice day!

yoko, 202301
