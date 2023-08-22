<p align="center">
<a title="logo" target="_blank" href="https://github.com/q191201771/lal">
<img alt="Live And Live" src="https://pengrl.com/lal/_media/lallogo.png">
</a>
<br>
<a title="Platform" target="_blank" href="https://github.com/q191201771/lal"><img src="https://img.shields.io/badge/platform-linux%20%7C%20macos%20%7C%20windows-green.svg"></a>
<a title="Release" target="_blank" href="https://github.com/q191201771/lal/releases"><img src="https://img.shields.io/github/tag/q191201771/lal.svg?label=release"></a>
<a title="CI" target="_blank" href="https://github.com/q191201771/lal/actions/workflows/ci.yml"><img src="https://github.com/q191201771/lal/actions/workflows/ci.yml/badge.svg?branch=master"></a>
<a title="goreportcard" target="_blank" href="https://goreportcard.com/report/github.com/q191201771/lal"><img src="https://goreportcard.com/badge/github.com/q191201771/lal"></a>
<a title="license" target="_blank" href="https://github.com/q191201771/lal/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-MIT-brightgreen.svg"></a>
<br>
<a title="wechat" target="_blank" href=""><img src="https://img.shields.io/:微信-q191201771-blue.svg"></a>
<a title="qqgroup" target="_blank" href=""><img src="https://img.shields.io/:QQ群-635846365-blue.svg"></a>
<br>
</p>

---

> [lal github address](https://github.com/q191201771/lal): https://github.com/q191201771/lal
>
> - ▦ I. Introduction to lalserver
> - ✒ lalserver features
> - ✒ Protocol conversion support
> - ▦ II. Installing and running lalserver
> - ✒ Method 1: Installing and compiling the source code
> - ✒ Method 2: Using a Docker image
> - ✒ Method 3: Downloading a pre-compiled binary executable
> - ▦ III. Using lalserver
> - ✒ List of push and pull stream URL addresses for each protocol
> - ✒ lalserver configuration file
> - ✒ lalserver HTTP events and interfaces
> - ✒ lalserver advanced
> - ▦ IV. Reacquainting yourself with lal
> - ✒ lal's three-tier structure
> - ✦ Other demos
> - ✦ Streaming stack library package/library
> - ✦ Golang common base library-naza
> - ✒ lalext
> - ✦ WebRTC
> - ✦ MCU Merge Streaming
> - ✒ In progress
> - ▦ V. Contact the author
> - ▦ VI. English translation

### ▦ I. Introduction to lalserver

[lalserver](https://github.com/q191201771/lal) is a pure Go streaming media server (live audio and video network transmission). It supports protocols RTMP, RTSP(RTP/RTCP), HLS, HTTP[S]/WebSocket-FLV/TS, [GB28181](https://www.chinesestandard.net/PDF/BOOK.aspx/GBT28181-2016) (Chinese National Standard). It also supports secondary development and expansion via plug-ins.

![lal feature map](_media/lal_feature.jpeg?date=220501)

#### ✒ lalserver features

- [x] **All platforms**
  - [x] Supports Linux/macOS/Windows multi-system development, debugging, running.
  - [x] Supports running under multiple architectures. For example, amd64/arm64/arm32/ppc64le/mipsle/s390x.
  - [x] Supports cross-compilation. You can compile executables for other platforms on any platform.
  - [x] Runs without dependencies. The executable file does not have any environment or library installation dependencies, and can run independently with a single file.
  - [x] Provides executables for all platforms, which can be run directly without compilation (provided that all the source code is open).
  - [x] Supports docker both for the amd and arm architectures.
- [x] **High performance**. Multi-core and multi-threaded expansion, support for high concurrency, performance and the same type of `c/c++` development services at the same level.
- [x] **High availability**. Perfect unit test. And there are multiple online environment applications.
- [x] **Multiple live streaming encapsulation protocols**.
  - [x] Supports RTMP, RTSP(RTP/RTCP), HLS, HTTP[S]/WebSocket-FLV/TS, GB28181 (Chinese National Standard).
  - [x] Supports conversion between different encapsulation protocols.
  - [x] Supports secondary development through plug-ins, support for expansion for custom protocols for input and output.
- [x] **Multiple encoding formats**. Video support: H.264/AVC, H.265/HEVC, audio support: AAC, G711A/G711U.
- [x] **Multi-format recording**. Supports FLV, long MPEGTS, HLS recording (HLS live and recording can be turned on at the same time).
- [x] **HTTPS**. Supports HTTPS-FLV, HTTPS-TS, HLS over HTTPS pull streaming.
- [x] **WebSocket**. Supports WebSocket-FLV, WebSocket-TS pull streams.
- [x] **TLS/SSL**. All protocols support corresponding TLS/SSL encrypted transmission. For example, RTMPS, RTSPS, HTTPS, WebSockets.
- [x] **HLS**. Supports LIVE real-time broadcasting and VOD full list broadcasting. Sliced files support multiple deletion methods. Supports memory slicing. Supports counting the number of HLS players.
- [x] **RTSP**. Support over TCP (interleaved mode). Supports basic/digest auth authentication. Supports 461 error code..
- [x] **RTMP**. Supports adding muted audio data to a single video, and merge sending. Compatible with various common RTMP implementations. Supports enhanced RTMP.
- [x] **RTP**. Supports parsing extended headers.
- [x] **HTTP API Interface**. Used to get information about a service and send commands to the server.
- [x] **HTTP Notify event callback**. Get time notification, business side can easily customise its own logic.
- [x] **Supports multiple methods of authentication**.
- [x] **Support for distributed clustering**
- [x] **Static pull back to source**. Configure back to source via config file.
- [x] **Static push forward**. Supports push forward to multiple addresses. Configure push forward addresses via config file.
- [x] **Support on-demand sourceback**. Back to source pull streams without an audience can be actively turned off.
- [x] **CORS cross-domain**. HTTP-FLV, HTTP-TS, HLS cross-domain pull streams are supported. HTTP-API supports cross-domain.
- [x] **HTTP file server**. For example, HLS sliced files can be played directly without additional HTTP file servers.
- [x] **Listening port multiplexing**. HTTP-FLV, HTTP-TS, HLS can use the same ports. The same applies to HTTPS.
- [x] **Second on playback**. GOP buffering.
- [x] **Supports (on server-side) automatic overlay of muted audio**.
- [x] **Supports pulling streams before pushing streams**.
- [x] **Supports reconnect after push stream disconnect, no gap perception on pull stream side**.
- [x] **Supports custom debug format stream recording, stream playback, quickly reproduce and locate the problem**.
- [x] **Bring your own Web UI page**.
- [x] **Supports video with B-frames**.

#### ✒ Protocol conversion support

See: [Appendix - Inter-Protocol Conversion Support](appendix.md)

### ▦ II. Installing and running lalserver

lalserver supports 3 methods for installation and subsequent running:

#### ✒ Method 1: Installing and compiling the source code.

Download the source code:
```shell
$ git clone https://github.com/q191201771/lal.git
```
Compile:
```shell
$ cd lal
$ export GO111MODULE=on && export GOPROXY=https://goproxy.cn,https://goproxy.io,direct
$ make
```

> **Tips:**
> If compiling with an IDE such as GoLand, then compiling the `lal/app/lalserver` directory is sufficient.
> If you don't have a Go compiler installed, you can refer to ["Installing Golang on CentOS or macOS"](https://pengrl.com/p/34426/) 🇨🇳, and for Windows OS you can search for tutorials online.

Run:

```shell
$ ./bin/lalserver -c conf/lalserver.conf.json
```

*Note that Windows replaces the path separator `/` with `\`*.

#### ✒ Method 2: Using a Docker image.

Download the image:

```shell
$ docker pull q191201771/lal
```
Run the image:

```shell
$ docker run -it -p 1935:1935 -p 8080:8080 -p 4433:4433 -p 5544:5544 -p 8083:8083 -p 8084:8084 -p 30000-30100:30000-30100/udp q191201771/lal /lal/bin /lalserver -c /lal/conf/lalserver.conf.json
```

*If you want to build your own Docker image, refer to [Appendix - Building Your Own Docker Image](appendix.md#-building-your-own-docker-image)*

#### ✒ Method 3: Downloading a pre-compiled binary executable

lal provides compiled lal binary executables for  the Linux, macOS, and Windows platforms (in zip archive).  
Download from: [github lal latest release page](https://github.com/q191201771/lal/releases/latest)  
The downloaded file can be run as shown in method 1.

### ▦ iii. Using lalserver

#### ✒ List of push and pull stream URL addresses for each protocol

Once lalserver has started successfully, you can begin using it.  
As a streaming service, the main function is forwarding streaming data. For example:

Use `ffmpeg` to push RTMP streams:

```shell
$ ffmpeg -re -i demo.flv -c:a copy -c:v copy -f flv rtmp://127.0.0.1:1935/live/test110
```

Use ffplay to pull RTMP streams for playback:

```shell
$ ffplay rtmp://127.0.0.1/live/test110
```

> For more protocols, see: [lalserver list of push and pull stream url addresses by protocol](streamurllist.md)
> 
> For more third-party clients, see: [Summary of common push/pull streaming client information](CommonClient.md)

#### ✒ lalserver configuration file

If you want to learn more about lalserver's capabilities, take a look at: [lalserver configuration file description](ConfigBrief.md).

#### ✒ lalserver HTTP Events and Interfaces

With the rich HTTP interfaces provided by lalserver, businesses can easily customise the system to fit their corporate logic using their own familiar language while keeping lalserver running independently.

- [lalserver HTTP Notify(Callback/Webhook) event callback](HTTPNotify.md)
- [lalsrver HTTP API Interface](HTTPAPI.md)

#### ✒ lalserver advanced

- [lalserver Authentication Anti-Leeching Link](auth.md)
- [lalserver secondary development - pub access to custom streams](customize_pub.md)

### ▦ IV. Getting reacquainted with lal

#### ✒ lal's three-tier structure

![lal source code architecture diagram](_media/lal_src_fullview_frame.jpeg?date=211211)

##### ✦ Other demos

In the lal project, in addition to `/app/lalserver`, which is the core service, there are some additional small applications provided in the `/app/demo` directory, such as push and pull streaming clients, as well as a pressure testing tool, a streaming analysis tool, and a scheduling example program for the lalserver cluster.  
These demos can be used both directly, and as examples to show you how the stack provided by lal can be used.

Learn more at: [Demo Introduction](DEMO.md)

##### ✦ Streaming stack library package/library

There are protocol stacks for both the client and server side.  
The protocol stacks in lal are all independent and designed in multiple layers.  
Businesses can integrate lal's protocol stack package library in their own applications.

##### ✦ Golang Common Base Library: naza

lal abstracts non-streaming-specific generic base libraries in a separate GitHub repo: [naza](https://github.com/q191201771/naza).

#### ✒ lalext

Standing on the shoulders of giants is the only way to see further! lal has combined lal's code with third-party libraries in another GitHub repo — [lalext](https://github.com/q191201771/lalext) — with a richer feature set.

##### ✦ WebRTC

A gateway to stream from RTMP to WebRTC.

Learn more at the [lalext GitHub address](https://github.com/q191201771/lalext).

##### ✦ SRT

#### ✒ In progress

- [TODO Roadmap - Summary being messed with](https://github.com/q191201771/lal/issues/157)
- [Indefinite delay - Summary that I'm not sure when I'll get to](https://github.com/q191201771/lal/issues/37)
- [Help wanted - Summary](https://github.com/q191201771/lal/issues/161)

### ▦ V. Contact the author

- WeChat：q191201771
- QQ: 191201771
- WeChat group: Add me on WeChat as a friend, and ask me to add you into the group!
- QQ group: 635846365
- E-mail: 191201771@qq.com
- lal github address: https://github.com/q191201771/lal
- lal official document: https://pengrl.com/lal

QR code to add the author on WeChat as a friend:

![lal author wechat](https://pengrl.com/images/yoko_vx.jpeg?date=220329)

Any technical and non-technical communication is welcome.

### ▦ VI. English translation

The English translation is currently a two-step process, and may not be in sync with the main Chinese documentation, which for all intents and purposes should _always_ remain authoritative.

Since the current human translator does not speak any Chinese, this requires machine translation via DeepL (possibly with some help from Google Translate as well) on the first step. Chinese <-> English technical translations work surprisingly well, but a second step is still needed to make the English more fluent and idiomatic (and less "robotic"!). Unfortunately, I'm afraid that [Chef/yoko](https://github.com/q191201771/)'s personal style will be lost in the process, and I apologise in advance. Perhaps one day a native Chinese speaker will be able to go through all the text and subtly correct the translation!

Due to the nature of the underlying publishing technology, there is no easy way to automate the whole process (at least for now), which means that occasionally some pages may be _very_ out of sync.