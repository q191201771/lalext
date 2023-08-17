# LAL v0.32.0发布，更好的支持纯视频流

Go语言流媒体开源项目 [LAL](https://github.com/q191201771/lal) 今天发布了v0.32.0版本。距离上个版本刚好一个月时间，[LAL](https://github.com/q191201771/lal) 依然保持着高效迭代的状态。

> LAL 项目地址：https://github.com/q191201771/lal

挑三个比较重要的修改简单介绍一下：

#### ▌ 一，自动叠加静音音频

这个是为了更好的支持 [纯视频流](https://pengrl.com/lal/#/concept?id=%e7%ba%af%e9%9f%b3%e9%a2%91%e6%b5%81%ef%bc%8c%e7%ba%af%e8%a7%86%e9%a2%91%e6%b5%81) ，解决很多播放器播放纯视频的直播流时，可能出现首帧打开时间特别慢、延迟大的问题。

详情我单独写了一篇文档： [《lalserver 自动叠加静音音频》](https://pengrl.com/lal/#/dummy_audio)

#### ▌ 二，支持RTMPS，RTSPS

基于TLS/SSL对RTMP和RTSP协议进行加密，带来更高的安全性。

RTMPS和RTSPS可以通过 [lalserver](https://github.com/q191201771/lal) 的配置文件开启，比如RTMPS对应的配置：

```
  "rtmp": {
    //. 是否开启rtmp服务的监听
    //  注意，配置文件中控制各协议类型的enable开关都应该按需打开，避免造成不必要的协议转换的开销
    "enable": true,

    //. RTMP服务监听的端口，客户端向lalserver推拉流都是这个地址//. RTMP服务监听的端口，客户端向lalserver推拉流都是这个地址
    "addr": ":1935",

    //. 是否开启rtmps服务的监听
    //  注意，rtmp和rtmps可以任意开启一个或全部打开或全部关闭
    "rtmps_enable": true,

    //. RTMPS服务监听的端口地址
    "rtmps_addr": ":4935",

    //. RTMPS的本地cert文件 
    "rtmps_cert_file": "./conf/cert.pem",

    //. RTMPS的本地key文件
    "rtmps_key_file": "./conf/key.pem",
    ...
  },
```

> 提示，以上内容来源 [《lalserver配置文件说明》](https://pengrl.com/lal/#/ConfigBrief) ，打开源文档可以获得更友好的阅读格式。

#### ▌ 三，更好的支持更多的RTSP摄像头

比如：

- rtp协议栈支持解析header中的padding和csrc
- 修复aac rtp type不是标准值导致无法合帧的问题。提高兼容性
- 修复rtsp auth可能失败的bug

以上处理都是对社区真实反馈的响应。

#### ▌ 更多修改

还有一些修改不逐个介绍了，大致如下：

> - [feat] demo: pullhttpflv拉取http-flv时可以存储为flv文件
> - [opt] 二次开发: 当DelCustomizePubSession后，调用被删除对象的FeedAvPacket方法将返回错误
> - [opt] 二次开发: 支持直接使用json字符串作为配置内容初始化ILalServer
> - [opt] 兼容性优化。转ts时，如果调整时间戳失败则使用调整前的时间戳。
> - [opt] 兼容性优化。当rtmps和rtsps加载签名文件失败时，只打印日志而不退出lalserver
> - [fix] http-api: 修复sub http-flv remote_addr字段没有值的bug
> - [log] 打印rtsp信令。丰富多处错误日志，比如转hls异常
> - [doc] 新增文档：重要概念 https://pengrl.com/lal/#/concept
>
> 以上内容摘取自 [《lal CHANGELOG版本日志》](https://pengrl.com/lal/#/CHANGELOG) ，你可以通过源文档获取更详细的内容。

#### 进一步了解lal

- [github](https://github.com/q191201771/lal)
- [官方文档](https://pengrl.com/lal)
- [联系作者](https://pengrl.com/lal/#/Author)


本文完，祝你今天开心。

yoko, 202211
