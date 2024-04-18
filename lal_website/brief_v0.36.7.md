# LAL v0.36.7发布，Customize Sub，我有的都给你

> **注意，v0.36.7之后的版本没有单独的发版介绍文档，修改内容见 [CHANGELOG.md](https://pengrl.com/lal/#/CHANGELOG)**

Go语言流媒体开源项目 [LAL](https://github.com/q191201771/lal) 今天发布了v0.36.7版本。

> LAL 项目地址：https://github.com/q191201771/lal

老规矩，简单介绍一下：

▦ Customize Sub，我有的都给你

这是提供给用lalserver做二次开发的小伙伴们的一个重要的功能，业务方可以通过设置回调函数的方式获取lalserver内部的流。

获取到流有什么用呢，拿lal的衍生项目lalmax来举例，它基于lalserver的Customize Sub，为所有输入lalserver中的流扩展了srt和webrtc播放的能力。

这里也简单介绍一下lalmax，它将会是lal整体版图中的一个重要项目，它基于lalserver提供的二次开发接口，与第三方库结合，支持更多的协议、实现更丰富的功能。  
相应的，lal项目会持续保持自身无第三方依赖，高性能等特点。  
lalmax项目地址： https://github.com/q191201771/lalmax

如果你想更好的理解Custmize Sub，可以先看看与Customize二次开发相关的另外两篇文档：

- [2.6 lalserver 二次开发(插件化)](https://pengrl.com/#/customize)
- [|-- 2.6.1 二次开发-pub接入自定义流](https://pengrl.com/#/customize_pub)

如果你想直接看代码：

- lal中相应的API签名： https://github.com/q191201771/lal/blob/master/pkg/logic/logic.go#L43
- lalmax使用示例： https://github.com/q191201771/lalmax/blob/master/main.go#L39

▦ rtmp兼容性优化

- amf编码中有两个不包含有效内容、没什么实际卵用的格式叫Undefined和Unsupported，之前lal没有解析，而现实中竟然真的有客户端会用它。
- 兼容publish信令中没有pubType字段的情况。是的，有的客户端就是这么任性，关键用的人还不少，不兼容还不行，气不气。
- 处理音频格式写在metadata中，而非单独seq header包的情况。
- Rtmp2AvPacketRemuxer增加参数用于指定是否需要丢弃SEI

▦ rtsp优化

- 支持处理时间戳翻滚的情况。也就是时间戳递增到一个非常大的数后，重新回到一个小的数继续递增。
- package rtsp中新增加了一个配置变量BaseInSessionTimestampFilterFlag，用于决定rtsp合成的帧级别packet的时间戳是使用rtp中的时间戳，还是重置成从0开始。

▦ lalserver优化

- 将NotifyHandler回调异步化到独立协程中。目的是方便业务方在回调中自由的调用lalserver的其他API接口
- http api中的流信息中增加fps字段
- 在业务方WithOnHookSession的情况下，停用auto stop relay pull功能

▦ bug修复

首先是一个比较严重的bug，会导致hls无法播放，这是在上个版本v0.35.4引入的，原因是转ts时，pmt中的avc标志笔误写错了写成aac了。。

然后是协议处理上的bug：

- rtp: 修复解析ext扩展数据的bug
- remux: Rtmp2AvPacketRemuxer多slice时append sps错误导致花屏
- rtmp2mpegts: 确保pts有值
- rtmp: 当ClientSession配置项WriteChanSize为0时，不必要开启异步发送

lalserver中释放资源的bug：

播放不存在的rtsp流，超时没有彻底释放

其他一些小bug：

使用hls中的回调对象IMuxerObserver前，先检查是否为nil

▦ 更多

还有一些修改不逐个介绍了，大致如下：

> - [chore] all shell file go to script folder
> - [chore] 所有脚本+x增加执行权限，保证CI正常运行
> - [chore] 修复macos平台readlink没有-f参数导致脚本执行失败的问题
> - [fix] webui: read null when no group
> - [fix] connection: not working set ModWriteChanSize
> - [fix] not working timout for RTMP server session
> - [test] dump rtsp test support video
> - [refactor] 整理所有跨域的代码
> - [refactor] 整理所有超时相关的代码
> - [refactor] hevc: 暴露hevc.Context中的所有字段
> - [refactor] avc: 暴露avc.Context中的Sps结构体字段
>
> 以上内容摘取自 [《lal CHANGELOG版本日志》](https://pengrl.com/lal/#/CHANGELOG) ，你可以通过源文档获取更详细的内容。

▦ 开发者

感谢参与这个版本的开源贡献者：yoko(阿亮), ZSC714725(阿响), HustCoderHu(小虎), Jae-Sung Lee(阿韩)

▦ 进一步了解lal

- [github](https://github.com/q191201771/lal)
- [官方文档](https://pengrl.com/lal)
- [联系作者](https://pengrl.com/lal/#/Author)

微信扫码加我好友（进微信群）：

![wechat](https://pengrl.com/images/yoko_vx.jpeg?date=2304)

本文完，祝你今天开心。

yoko, 202307
