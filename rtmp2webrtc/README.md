### rtmp2webrtc

rtmp转换webrtc的网关。

通过它，你可以使用h5网页通过WebRTC协议播放已有的RTMP流（RTMP流可以是任意支持RTMP拉流的服务器上的流）。使用方法：

## Linux依赖库
```
sudo apt install libavformat-dev libswresample-dev libavcodec-dev libavutil-dev
```

步骤1，编译并运行协议转换程序

```
$cd rtmp2webrtc
$go build
$./rtmp2webrtc -p 8827 -wp 8900
```
也可以使用`./rtmp2webrtc -p 8827 -wp 8900 -ip x.x.x.x`，`x.x.x.x`是你机器ip地址。这样不用依赖外部stun服务，也可以减少建链时间。 

步骤2，使用chrome浏览器打开h5网页： http://127.0.0.1:8827/rtmp2webrtc.html

步骤3，在网页中`rtmp url:`下面的输入框中填入可正常拉流的RTMP地址。点击`Start Session`按钮。完成，正常情况下视频就可以播放出来了。

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

