## Protocol Support List

| Push Stream Client | RTMP | RTSP |
|--------------------|------|------|
| ffmpeg             |  ✔   |  ✔   |
| obs                |  ✔   |  X   |

| Pull Streaming Client  | RTMP | RTSP | HTTP-FLV | HTTPS-FLV | HLS | HTTP-TS | WebSocket-FLV | WebSocket-TS | Remarks |
|------------------------|------|------|----------|-----------|-----|---------|---------------|--------------|---------|
| ffmpeg/ffplay          |  ✔   |  ✔   |    ✔     |     ✔     |  ✔  |    ✔    |       X       |       X      |         |
| vlc                    |  ✔   |  ✔   |    ✔     |     X     |  ✔  |    ✔    |       X       |       X      |         |
| MPV                    |  ✔   |  ✔   |    ✔     |     ✔     |  ✔  |    ✔    |       X       |       X      |         |
| flv.js                 |  X   |  X   |    ✔     |     ?     |  X  |    X    |       ✔       |       X      |         |
| hls.js                 |  X   |  X   |    X     |     X     |  ✔  |    X    |       X       |       X      |         |
| Watermelon Player (js) |  X   |  X   |    ✔     |     ?     |  ✔  |    X    |       ✔       |       X      |         |
| MPlayer                |  ?   |  ?   |    ?     |     ?     |  ?  |    ?    |       ?       |       ?      |         |

## Some client instructions

TODO(chef): This part needs to be sorted out.

* 1 rtmp push stream
  * 1.1 obs
    * 1.1.1 obs capture camera
  * 1.2 ffmpeg
    * 1.2.1 ffmpeg read file push
* 2 rtsp push stream
  * 2.1 ffmpeg
    * 2.1.1 ffmpeg read file push streaming
* 3 rtmp pull stream
  * 3.1 vlc
  * 3.2 ffmpeg
    * 3.2.1 ffmpeg pull stream save file
  * 3.3 ffplay
  * 3.4 mpv
* 4 `http-flv` and `https-flv` pull streams
* 5 `HLS (m3u8+ts)` pull-streaming
  * 5.1 safari
  * 5.2 vlc, ffmpeg, ffplay
* 6 `http-ts` pull-streaming
* 7 rtsp pull-streaming
* 8 test file download

(Followed by adding other clients to use, and other streaming protocol formats.)

## 1 rtmp push stream

### 1.1 OBS

Go to the official website (https://obsproject.com/) to download the binary installation package for the corresponding operating system and install it.

This article was written using a macOS system, running OBS version 25.0.8, but other systems and OBS versions are similar.

#### 1.1.1 obs capture camera

- Open obs, click the **Settings** button in the lower right corner.
- In the pop-up window, click **Push Stream** button on the left side.
- The **Push Stream** details page appears on the right side:
~ Leave the service dropdown box alone, keep it customised...
- Fill in `rtmp://127.0.0.1:1935/live` in the server input box.
- Fill in `test110` in the streaming key input box.
- By default, the streaming key does not show the input characters in plaintext, if you are afraid of outputting them, you can click the Show button on the right side.
- Click the Confirm button at the bottom right corner to complete the setup.
- Go back to the main interface, click the **Start Streaming** button at the bottom right corner.
- The status bar at the bottom shows the information of LIVE, fps, kb/s, etc., which indicates the duration of streaming, frame rate and bit rate respectively, which means the streaming is successful.

OBS can not only capture camera as input stream, but also provide desktop, audio and video file capture and other functions. And it provides a lot of parameters that can be configured on the graphical interface.

### 1.2 ffmpeg

#### 1.2.1 ffmpeg read file push

(How to install ffmpeg — see the end of the article)

There are two cases here, one is that the audio in the FLV and MP4 files is itself in AAC format, and the video itself is in H.264 or H.265 format, which is supported by the streaming server itself. In this case, ffmpeg does not need to re-encode the audio and video:

```shell
$ ffmpeg -re -stream_loop -1 -i demo.flv -c:a copy -c:v copy -f flv rtmp://127.0.0.1:1935/live/test110
```

A few words about the parameters:

`-re` means that the audio and video streams are pushed according to the bit rate of the file, if you do not add it, you do not control the sending speed and send it to the server at once, which is not in line with the characteristics of the live broadcast.  
``-stream_loop -1`` means that after the end of the file, the number of times to continue to push from the head of the file loop, `-1` means infinite loop.  
`-i` means input file.  
`-c:a copy` means the audio encoding format remains unchanged.   
`-c:v copy` means the video encoding format is unchanged.  
`-f flv` to push RTMP streams you need to specify the format as FLV.

Finally the RTMP push stream address:  
If it is an MP4 file, just replace `demo.flv` with an MP4 file name, such as `demo.mp4`.

The above is one of the more common cases.

Another situation is when the file in the audio and video encoding format is not supported by the streaming media server. In that case, ffmpeg need to re-encode the stream:

```shell
$ ffmpeg -re -i demo.flv -c:a aac -c:v h264 -f flv rtmp://127.0.0.1:1935/live/test110
```
Where `-c:a aac` means audio is encoded with AAC and `-c:v h264` means video is encoded with H.264.

## 2 rtsp push stream

### 2.2 ffmpeg

#### 2.2.1 ffmpeg read file push

```shell
$ ffmpeg -re -stream_loop -1 -i demo.flv -acodec copy -vcodec copy -f rtsp rtsp://localhost:5544/live/test110
```

In addition, RTSP also supports RTP over TCP to push stream, the corresponding `ffmpeg` command is as follows:

```shell
$ ffmpeg -re -stream_loop -1 -i demo.flv -acodec copy -vcodec copy -rtsp_transport tcp -f rtsp rtsp://localhost:5544/live/test110
```

See 1.2.1 for the meaning of the parameters.

## 3 rtmp stream pulling

### 3.1 vlc

Go to the official website (https://www.videolan.org/vlc/) to download the binary installation package for the corresponding operating system and install it.

In this article, we use VLC 3.0.8 for macOS as a demo, other systems and VLC versions are similar.

- Open VLC.
- Click **File** in the bottom menu bar, then click **Open Network...**.
- Enter `rtmp://127.0.0.1:1935/live/test110` in the URL input box in the pop-up window.
- Click the **Confirm** button in the lower right corner to complete the setup.
- Start playing the live stream.

### 3.2 ffmpeg

#### 3.2.1 ffmpeg pull stream save file

See the end of this article for how to install `ffmpeg`.

```shell
$ ffmpeg -i rtmp://127.0.0.1:1935/live/test110 -c copy test110.flv
```

### 3.3 ffplay
`ffplay` is a player that comes with the `ffmpeg` project.

```shell
$ ffplay rtmp://127.0.0.1:1935/live/test110
```

### 3.4 mpv

Download the binary installer from the official website, install it, and then input the streaming address to play it, which is basically the same as VLC, refer to **2.1 vlc**.

## 4 http-flv and https-flv streaming

HTTP-FLV pull stream, for VLC, ffmpeg, ffplay, mpv, and pull RTMP stream is the same, change the pull stream URL from  
`rtmp://127.0.0.1:1935/live/test110`  
to  
`http://127.0.0.1:8080/live/test110.flv`

You can do it!

The HTTPS-FLV pull stream is the same as HTTP-FLV.

## 5 HLS(m3u8+ts) pull streaming

### 5.1 safari browser

Open safari browser, input HLS streaming address in the address line:  
`http://127.0.0.1:8080/hls/test110/playlist.m3u8`

You can do it!

### 5.2 vlc, ffmpeg, ffplay

HLS(m3u8+ts) stream pulling, for VLC, ffmpeg, ffplay, it is the same as pulling RTMP streams, change the stream pulling URL from  
`rtmp://127.0.0.1:1935/live/test110`  
to  
`http://127.0.0.1:8080/hls/test110/playlist.m3u8`

## 6 http-ts streaming

HTTP-TS stream pulling, for VLC, ffmpeg, ffplay, mpv, is the same as pulling RTMP streams, change the stream pulling URL from  
`rtmp://127.0.0.1:1935/live/test110`  
to  
`http://127.0.0.1:8080/live/test110.ts`

## 7 rtsp streaming
```shell
$ ffplay rtsp://localhost:5544/live/test110
$ ffmpeg -i rtsp://localhost:5544/live/test110 -vcodec copy -acodec copy -f flv /tmp/test110.flv
$ ffmpeg -rtsp_transport tcp -i rtsp://localhost:5544/live/test110 -vcodec copy -acodec copy -f flv /tmp/test110.flv
```

## 8 Test File

Download the test file:

https://github.com/q191201771/doc

The test file conversion generates the command:
```shell
$ ffmpeg -i wontcry.mp4 -acodec aac -vcodec h264 -r 15 -g 30 -keyint_min 30 -bf 0 -f flv wontcry.flv
```
`-r 15` FPS frame rate, means encode 15 frames in 1 second.  
`-g 30` GOP size, i.e. I-frame interval, 1 I-frame every 30 frames, i.e. 1 I-frame every 2 seconds.  
`-keyint_min 30` Minimum GOP size.  
`-bf 0` Do not use B-frame encoding.  
`-acodec aac` Use AAC for audio encoding in output files.  
`-vcodec h264` Use H.264 for video encoding of output files.  
`-i wontcry.mp4` Input file.  
`-f flv wontcryflv.flv` Output FLV file.

```shell
$ ffmpeg -i wontcry.flv -acodec copy -vcodec copy -t 30 -f flv wontcry30s.flv
```
`-t 30` Capture the first 30 seconds.

```shell
$ ffmpeg -i wontcry30s.flv -acodec aac -vcodec hevc -r 15 -g 30 -keyint_min 30 -bf 0 -preset ultrafast -x265-params "bframes=0" -f flv hevc.flv
```
Transcode to H.265 without B-frames.

## ffmpeg installation

- [Linux install ffmpeg](https://pengrl.com/p/20029/) 🇨🇳
- [macOS compile Kingsoft Cloud ksvc ffmpeg to support HEVC H.265 on top of RTMP FLV](https://pengrl.com/p/20044/) 🇨🇳
- [macOS compile ffmpeg in player ffplay](https://pengrl.com/p/20043/) 🇨🇳
- [macOS Catalina 10.15.1 environment — compiled `ffmpeg` can not run, crashes on startup](https://pengrl.com/p/20042/) 🇨🇳

*Translator's note: These are blog entries on yoko's personal blog and have not been translated.*

yoko, 20210206

---

Being original is not easy. Reproduction is allowed, but please reference this article as being part of the documentation for the open-source streaming media server [lal](https://github.com/q191201771/lal) (GitHub link).   Official documentation: [https://pengrl.com/lal](https://pengrl.com/lal)