# Appendix

## ✒ Protocol conversion support

| Trans encapsulation type | sub rtmp | sub http[s]/websocket[s]-flv | sub http[s]/websocket[s]-ts | sub hls | sub rtsp | relay push rtmp |
|--------------------------|:--------:|:----------------------------:|:---------------------------:|:-------:|:--------:|:---------------:|
| pub rtmp                 |    ✔     |             ✔                |              ✔              |    ✔    |    ✔     |       ✔         |
| pub rtsp                 |    ✔     |             ✔                |              ✔              |    ✔    |    ✔     |       ✔         |
| relay pull rtmp          |    ✔     |             ✔                |              ✔              |    ✔    |    ✔     |       ✔         |

### Support for encoding protocols by encapsulation protocol

| Encoding type | rtmp | rtsp | hls | flv | mpegts |
|---------------|:----:|:----:|:---:|:---:|:------:|
| aac           |   ✔  |   ✔  |  ✔  |  ✔  |   ✔    |
| avc/h264      |   ✔  |   ✔  |  ✔  |  ✔  |   ✔    |
| hevc/h265     |   ✔  |   ✔  |  ✔  |  ✔  |   ✔    |

### Recorded file type

| Type of recording | hls | flv | mpegts |
|-------------------|:---:|:---:|:------:|
| pub rtmp          |  ✔  |  ✔  |    ✔   |
| pub rtsp          |  ✔  |  ✔  |    ✔   |

For the meaning of this table, see: [connection type session pub/sub/push/pull](Session.md).

Note that if only RTSP streams (RTP packets to be exact) are forwarded to each other, and no transcapsulation into other formats is involved, all types are theoretically supported.

## ✒ Building your own Docker image
Generate an image based on local code and Dockerfile file and run it:

Download the source code:
```shell
$ git clone https://github.com/q191201771/lal.git
```

Build the image:
```shell
$ cd lal
$ docker build -t lal .
```
Run the image:
```shell
$ docker run -it -p 1935:1935 -p 8080:8080 -p 4433:4433 -p 5544:5544 -p 8083:8083 -p 8084:8084 -p 30000-30100:30000-30100/udp lal /lal/bin/lalserver - c /lal/conf/lalserver.conf.json
```
