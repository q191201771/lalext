# rtmp2webrtc

## 🇨🇳

rtmp转换webrtc的网关。

通过它，你可以使用h5网页通过WebRTC协议播放已有的RTMP流（RTMP流可以是任意支持RTMP拉流的服务器上的流）。使用方法：

步骤1，编译并运行协议转换程序

```sh
$ cd rtmp2webrtc
$ go build
$ ./rtmp2webrtc -p 8827 -wp 8900
```

参数说明：

-p 8827 是HTML页面的TCP端口
-wp 8900 是WebRTCICE的端口，包含TCP和UDP端口

另外，也可以使用`./rtmp2webrtc -p 8827 -wp 8900 -ip x.x.x.x`，`x.x.x.x`是你机器ip地址。这样不用依赖外部stun服务，也可以减少建链时间。 

步骤2，使用chrome浏览器打开h5网页： http://127.0.0.1:8827/rtmp2webrtc.html

步骤3，在网页中`RTMP URL:`下面的输入框中填入可正常拉流的RTMP地址。点击`Start Session`按钮。完成，正常情况下视频就可以播放出来了。

注意：

- 目前只能播放rtmp中的H264格式的视频，音频会被过滤掉，H265不支持
- 如果失败：
  1. 确保你的rtmp流是正常的。比如可以通过ffmpeg、vlc等rtmp客户端正常播放
  2. 在没有指定`ip`参数时，确保网络能使用`stun.l.google.com:19302`
  3. 这只是一个演示协议转换的demo，不会过多的判断边界错误条件，你可以尝试重启rtmp2webrtc，重新刷新页面，更换新的流名称就行测试
- 我测试过的浏览器：macos下的chrome和firefox

### 注意

lalext项目是lal的附属项目，临时存放一些暂时不放入lal中的内容，所有如果lalext项目对你有用，建议你：

1. fork lalext项目进行存档
2. 关注lal github： https://github.com/q191201771/lal

### 依赖

- q191201771/lal
- pion/webrtc

### webrtc2rtmp

使用h5网页通过WebRTC协议推流至rtmp服务器。（实现中）

## 🇬🇧

### rtmp2webrtc

Gateway for RTMP to WebRTC conversion.

With it, you can use any HTML5 webpage to play existing RTMP streams via the WebRTC protocol (RTMP streams can be streams from any server that supports RTMP pull). 

#### Usage:

##### Step 1, compile and run the protocol converter

```sh
$ cd rtmp2webrtc
$ go build
$ ./rtmp2webrtc -p 8827 -wp 8900
```

Parameter description:

`-p 8827` is the TCP port for HTML pages.
`-wp 8900` is the port of WebRTCICE, including TCP and UDP ports.

Alternatively, you can use `./rtmp2webrtc -p 8827 -wp 8900 -ip x.x.x.x`, where `x.x.x.x` is your machine's IP address. This way you don't have to rely on an external [STUN](https://www.pcmag.com/encyclopedia/term/stun) service, and you can also reduce the chain building time. 

##### Step 2, use the Chrome Browser to open the HTML5 webpage at http://127.0.0.1:8827/rtmp2webrtc.html

##### Step 3, in the input box under `RTMP URL:` in the webpage, fill in the RTMP address that is able to properly pull the stream.

Click the `Start Session` button. Done, the video will play as expected.

### Note 1

- Currently you can only play H.264 video in RTMP, audio will be filtered out, H.265 is not supported.
- If it fails:
  1. Make sure your RTMP stream is operational. For example, confirm that it can be played normally by `ffmpeg`, VLC, or any other RTMP client.
  2. Make sure the network can use `stun.l.google.com:19302` when the `ip` parameter is not specified.
  3. This is just a demo to show the protocol conversion, it won't check the boundary error conditions too extensively, you can try to restart `rtmp2webrtc`, refresh the page, and change the new stream name on the line to test!
- Browsers I tested: Chrome and Firefox on macOS.

### Note 2

**lalext** project is a subsidiary project of [lal](https://github.com/q191201771/lal), temporarily storing some content that has not made it into lal, so if the **lalext** project is useful to you, it is recommended that you:

1. Fork the lalext project to archive it;
2. Follow lal GitHub: https://github.com/q191201771/lal.

### Dependencies

- q191201771/lal
- pion/webrtc

### webrtc2rtmp

Use HTML5 webpage to push stream to RTMP server via WebRTC protocol. (Implementation in progress.)
