# LAL v0.33.0发布，支持抓取流数据回放调试

Go语言流媒体开源项目 [LAL](https://github.com/q191201771/lal) 今天发布了v0.33.0版本。

> LAL 项目地址：https://github.com/q191201771/lal

老规矩，先挑三个修改简单介绍一下：

#### ▌ 一，支持抓取流数据回放调试

这个功能可以大幅提高lal用户反馈问题、定位问题的效率，从而提高lal的兼容性。

详情我单独写了一篇文档： [《debug dump抓取lal流调试》](https://pengrl.com/lal/#/debug_dump)

#### ▌ 二，HTTP-API和HTTP-Notify回调为HLS协议提供更丰富的信息

我们都知道，HLS是客户端持续性发起一个又一个HTTP请求，每次获取一个TS流片段文件的方式来播放的。  
这种短连接的方法导致服务端不好区分不同的播放者，也不好判断播放者开始播放、结束播放的时间点。

此次，我们通过增加302跳转，给m3u8 url增加一个带用户唯一ID的参数，并且给一个用户的多个TS都增加上该ID，从而支持：

- 获取hls播放者数量、码率等信息
- 获取hls开始播放、结束播放的事件通知

这个功能在配置文件有两个相关的配置，具体见 [hls/sub_session_timeout_ms](https://pengrl.com/lal/#/ConfigBrief?id=%e2%9c%b8-quothlssub_session_timeout_msquot) 和 [hls/sub_session_hash_key](https://pengrl.com/lal/#/ConfigBrief?id=%e2%9c%b8-quothlssub_session_hash_keyquot)

如果你不喜欢m3u8 302跳转带来的开销，你也不关心hls的统计与通知的信息，那么你就可以通过配置文件将该功能关闭。

> 相关文档：  
> [《lalserver HTTP API接口》](https://pengrl.com/lal/#/HTTPAPI)  
> [《lalserver HTTP Notify(Callback/Webhook)事件回调》](https://pengrl.com/lal/#/HTTPNotify)

#### ▌ 三，解析rtp extension扩展头

增加了解析rtp扩展头的逻辑，使得可以更好的支持rtsp流。

#### ▌ 更多修改

还有一些修改不逐个介绍了，大致如下：

> - [chore] docker同时支持amd和arm架构
> - [feat] demo: analyseflv支持http flv流或flv文件作为输入
> - [feat] 插件化例子：增加读取flv文件再通过CustomPubSession将数据输入lalserver的例子
> - [opt] rtmp: 缩小打chunk时预分配的内存大小
> - [opt] 插件化：Cutsomize Pub支持AvPacket、RtmpMsg两种输入数据的方式
> - [opt] Gop缓冲功能支持配置单个Gop内的最大缓冲帧数量
> - [fix] 修复hls获取app name失败的问题
> - [fix] flv: 修复ReadAllTagsFromFlvFile中没有关闭文件的bug
> - [fix] rtmp: 接收buff解析前有效长度检查
>
> 以上内容摘取自 [《lal CHANGELOG版本日志》](https://pengrl.com/lal/#/CHANGELOG) ，你可以通过源文档获取更详细的内容。

#### 进一步了解lal

- [github](https://github.com/q191201771/lal)
- [官方文档](https://pengrl.com/lal)
- [联系作者](https://pengrl.com/lal/#/Author)


本文完，祝你今天开心。

yoko, 202301
