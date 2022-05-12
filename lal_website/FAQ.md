# lal FAQ

> FAQ 展示地址： https://pengrl.com/lal/#/FAQ
> FAQ 文档存放地址：github [lalext项目](https://github.com/q191201771/lalext) 的 [lal_website/FAQ.md](https://github.com/q191201771/lalext/blob/master/lal_website/FAQ.md)
>
> 欢迎大家添加自己的问题，或者自问自答，或者回答文档里最下面待整理的问题。  
> 
> 参与方式，可以提PR修改文档 [lal_website/FAQ.md](https://github.com/q191201771/lalext/blob/master/lal_website/FAQ.md)，也可以到这个 [issue](https://github.com/q191201771/lalext/issues/4) 里直接跟帖。

#### ▌

##### Q: 使用lalext/rtmp2webrtc时，观看画面卡住不动，画面跳跃？鬼畜？

##### Q: 为什么使用vlc等播放器播放h265 rtmp失败了？

因为rtmp标准不支持h265，所以原生的ffmpeg和vlc都无法直接播放h265 rtmp/flv，可以使用 [lalext](https://github.com/q191201771/lalext) 项目里`thirdparty/build.sh`编译一个支持h265的ffmpeg。

##### Q: 使用lalserver，推流结束后，vlc、ffplay等播放器卡在最后一帧画面不退出？

首先，lalserver有超时机制，推流结束一段时候后，会主动断开拉流播放端的连接。  
播放器卡在最后一帧画面不退出，和具体播放器对于server端关闭连接后的具体实现有关系。  
用户可通过lalserver的日志、netstat、tcpdump等手段来查询连接是否已被关闭。  

##### Q: 为什么默认配置中部分协议不使用对应的标准端口？比如HTTP不使用80端口，RTSP不使用554端口

因为Linux系统下，绑定1024以下的端口需要root权限，所以大部分开源项目的测试默认端口会使用类似8080，3000这种端口，而非80端口。  
用户在有权限的前提下，可以修改配置。

#### ▌ rtsp

##### Q: rtsp支持组播吗

目前不支持。后续会开发。 TODO

#### ▌ 应用场景

##### Q: lalserver能否做视频会议？

目前不支持。 TODO

##### Q: 在采集IPcamera 视频 AI处理然后推流的应用场景中， 这个框架怎样和AI处理衔接呢？

lal主要聚焦在传输方面，至于传输前和传输后对编码前、解码后的音视频如何处理，lal并不太关心。  
比如这个问题中所说的IPcamera AI处理后推的流，只需要封装成lal支持的标准协议格式，就能衔接使用。  

#### ▌ 待整理

##### Q: lalserver的插件，webrtc的规划

##### Q: rtsp支持RTP over TS和RTP over PS吗

##### Q: 每种协议支持的编码格式和封装格式

##### Q: 支持防盗链吗？

##### Q: 支持鉴权吗？

##### Q: 支持h265播放吗？

##### Q: 每种协议支持的编码格式和封装格式？

##### Q: 支持加密吗？

##### Q: 是否免费？

##### Q: 支持回放吗？

##### Q: obs推流到lalserver失败了，是什么问题，怎么排查？

##### Q: lal的延迟有多大？

##### Q: 海康威视监控摄像头，网页无法播放视频，现象：VLC 能够播放,但是等待时间略长 5秒左右，mpegts.js完全无法播放，西瓜播放器也无法播放。mpegts播放抓包，显示一直在进行数据拉取，时间轴一直在增加。但是始终无法播放。

(#153)

##### Q: (#150)

##### Q: (#126)