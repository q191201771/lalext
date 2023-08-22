# LAL v0.34.3 released, G711 audio is here, and so is the Web UI!

The Go language streaming open source project [LAL](https://github.com/q191201771/lal) released v0.34.3 today.

> LAL project address: https://github.com/q191201771/lal

The old rules — first, a brief introduction:

#### ▦ i. Audio G711

Added support for audio G711A/G711U (also known as PCMA/PCMU). The main manifestations are:

✒ 1) RTMP G711 as input

1.1) RTMP-in-RTMP-out:

The test method is as follows:

```bash
# Push stream command:
ffmpeg -re -i testdata/test.flv -vcodec copy -acodec pcm_mulaw -ac 1 -ar 8000 -f flv "rtmp://127.0.0.1/live/test110"

# play command:
ffplay rtmp://127.0.0.1/live/test110
```

Hint, `-acodec pcm_mulaw` is G711U, if you want to use G711A, then change it to `-acodec pcm_alaw`

1.2) Then there's the addition of RTMP to RTSP support

```bash
# push stream command:
ffmpeg -re -i testdata/test.flv -vcodec copy -acodec pcm_mulaw -ac 1 -ar 8000 -f flv "rtmp://127.0.0.1/live/test110"

# Playback commands:
ffplay rtsp://127.0.0.1:5544/live/test110
```

✒ 2) RTSP g711 as input

Enhanced support for RTSP-forwarding RTSP.

For RTSP-in-RTSP-out scenarios, lalserver only needs to do forwarding for rtsp data, which theoretically supports all encoding formats.  
But in practice, due to the lack of compatibility with SDP format, the support for rtsp2rtsp G711 is not good enough, and the new version has made enhancements.

```bash
# Push stream command:
ffmpeg -re -stream_loop -1 -i testdata//test.flv -acodec pcm_mulaw -ac 1 -ar 8000 -vcodec copy -f rtsp rtsp://localhost:5544/live/test110

# Playback commands:
ffplay rtsp://127.0.0.1:5544/live/test110
```

✒ 3) Follow-up

3.1) Current version of G711 RTSP to RTMP, only video without sound, we will support it as soon as possible, this work is already in progress.  
3.2) Enhance the performance of G711 under each sample rate.  
3.3) Add support for container protocols such as `ts` for G711.  

#### ▦ II. Web UI

Add a simple web status page inside the lalserver process. After starting lalserver, type http://127.0.0.1:8083/lal.html in the browser to access it.

[click here if you want to see screenshots](http_web_ui.md), which is basically divided into three main parts:

1. Basic information about the service
    - Basic information about the service, such as version, startup time, etc.
2. Group list information
    - For example, how many input streams are there in total?
    - Group list information, e.g. how many input streams there are in total, e.g. name of each stream, start time, remote address, video encoding format, aspect ratio, current bitrate, etc.
3. Specific group information
    - Operation mode: click the streams in the group list.
    - For example, how many streams are pulling, information of pulling streams, etc.

The page can be refreshed regularly by ticking Auto.

The page is rendered by lalserver's internal HTTP serve and it calls lalserver's HTTP-API  to get the information dynamically.  
Only native front-end technologies are used, no third-party JS/CSS libraries are introduced.

We upgraded the Go version of lal from 1.14 to 1.16 with minimal dependencies, so that we can use the embed feature to embed HTML pages into  lalserver executable.  
This means that you don't need to deploy HTML static files when deploying lalserver, just the lalserver executable plus any configuration files.

In the future, we will add all the features based on HTTP-API interface to this page, such as sending commands to pull rtsp streams from the remote end.

#### ▦ III. Applet lag

When WeChat applet pushes stream with rtmp, it supports a mode called RTC (Real Time Calling), and it will be very laggy when it pushes stream to lalserver and then pulls stream from lalserver to play after using this mode.  
This version fixes this bug.

#### ▦ More

There are some more modifications not to be introduced one by one, they are roughly as follows:

> - [feat] rtmp: server session support send ack
> - [opt] rtmp: client session only uses the value of PeerWinAckSize as a trigger for whether to send an ack or not
> - [refactor] remove deprecated io/ioutil packages
> - [opt] rtmp: ClientSession support for customised tls.Config
> - [fix] Fix a bug in parsing config files where the sub_session_hash_key field is written incorrectly
> - [log] add log when not caching frame by SingleGopMaxFrameNum
> - [chore] add version checking script check_versions.sh
> - [chore] add check_versions.sh
> 
> The above is extracted from ["lal version log"](../CHANGELOG.md), you can get more details from the source document.

#### ▦ Developers

Thanks to the open source contributors who worked on this release: yoko(Liang), ZSC714725(Ah Loud), jaesung9507(Ah Han), liquanqing(Ah Khong), joestarzxh(Ah Hai).  

#### ▦ Learn more about lal

- [GitHub](https://github.com/q191201771/lal)
- [Official Documentation](https://pengrl.com/lal)
- [Contact Author](https://pengrl.com/lal/#/Author)

WeChat scan QR code to add yourself as the author's friend (into the WeChat group):

![WeChat](https://pengrl.com/images/yoko_vx.jpeg?date=2303)

End of this article. Have a nice day!

yoko, 202303