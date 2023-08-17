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

> [lal github 地址](https://github.com/q191201771/lal)： https://github.com/q191201771/lal

> - ▦ 一. lalserver 简介
>   - ✒ lalserver 特性
>   - ✒ 协议转换支持情况
> - ▦ 二. lalserver 安装、运行
>   - ✒ 方式1, 源码编译安装
>   - ✒ 方式2, Docker镜像
>   - ✒ 方式3, 下载编译好的二进制可执行文件
> - ▦ 三. lalserver 使用
>   - ✒ 各协议推拉流url地址列表
>   - ✒ lalserver 配置文件
>   - ✒ lalserver HTTP事件和接口
>   - ✒ lalserver 进阶
> - ▦ 四. 重新认识lal
>   - ✒ lal的三层结构
>     - ✦ 其他demo
>     - ✦ 流媒体协议栈库package/library
>     - ✦ Golang通用基础库-naza
>   - ✒ lalext
>     - ✦ WebRTC
>     - ✦ MCU合流
>   - ✒ 进行中
> - ▦ 五. 联系作者

### ▦ 一. lalserver 简介

[lalserver](https://github.com/q191201771/lal)是纯Golang开发的流媒体（直播音视频网络传输）服务器。目前已支持RTMP, RTSP(RTP/RTCP), HLS, HTTP[S]/WebSocket-FLV/TS, GB28181协议。并支持通过插件形式进行二次开发扩展。

![lal特性图](https://pengrl.com/lal/_media/lal_feature.jpeg?date=220501)

#### ✒ lalserver 特性

- [x] **全平台**
  - [x] 支持linux/macOS/windows多系统开发、调试、运行
  - [x] 支持多arch运行。比如amd64/arm64/arm32/ppc64le/mipsle/s390x
  - [x] 支持交叉编译。可在任一平台编译出其他平台的可执行文件
  - [x] 运行无依赖。可执行文件无任何环境、库安装依赖，可单文件独立运行
  - [x] 提供各平台可执行文件，可免编译直接运行(前提是开放全部源码哈)
  - [x] 支持docker。支持amd和arm架构
- [x] **高性能**。多核多线程扩展，支持高并发，性能和同类型`c/c++`开发服务处于同一水平
- [x] **高可用**。完善的单元测试。并且有多个线上环境应用
- [x] **多种直播流封装协议**
  - [x] 支持RTMP, RTSP(RTP/RTCP), HLS, HTTP[S]/WebSocket-FLV/TS, GB28181
  - [x] 支持不同封装协议间相互转换
  - [x] 支持通过插件形式进行二次开发，支持扩展自定义协议，输入输出都可以
- [x] **多种编码格式**。视频支持H264/AVC，H265/HEVC，音频支持AAC，G711A/G711U
- [x] **多种格式录制**。支持FLV，长MPEGTS，HLS录制(HLS直播与录制可同时开启)
- [x] **HTTPS**。支持HTTPS-FLV，HTTPS-TS，HLS over HTTPS拉流
- [x] **WebSocket**。支持Websocket-FLV，WebSocket-TS拉流
- [x] **TLS/SSL**。所有协议都支持相应的TLS/SSL加密传输。比如RTMPS，RTSPS，HTTPS，WebSockets
- [x] **HLS**。支持LIVE实时直播、VOD全列表直播。切片文件支持多种删除方式。支持内存切片。支持统计HLS播放者数量
- [x] **RTSP**。支持over TCP(interleaved模式)。支持basic/digest auth验证。支持461。
- [x] **RTMP**。支持给单视频添加静音音频数据，支持合并发送。兼容对接各种常见RTMP实现。支持enhanced RTMP。
- [x] **RTP**。支持解析扩展头
- [x] **HTTP API接口**。用于获取服务信息，向服务发送命令。
- [x] **HTTP Notify事件回调**。获取时间通知，业务方可以轻松定制自身的逻辑。
- [x] **支持多种方式鉴权**
- [x] **支持分布式集群**
- [x] **静态pull回源**。通过配置文件配置回源地址
- [x] **静态push转推**。支持转推多个地址。通过配置文件配置转推地址
- [x] **支持按需回源**。没有观众的回源拉流可以主动关闭
- [x] **CORS跨域**。支持HTTP-FLV，HTTP-TS，HLS跨域拉流。HTTP-API支持跨域。
- [x] **HTTP文件服务器**。比如HLS切片文件可直接播放，不需要额外的HTTP文件服务器
- [x] **监听端口复用**。HTTP-FLV，HTTP-TS，HLS可使用相同的端口。over HTTPS类似
- [x] **秒开播放**。GOP缓冲
- [x] **支持(在服务端)自动叠加静音音频**
- [x] **支持先拉流后推流**
- [x] **支持推流断开后重连，拉流端无感知**。
- [x] **支持自定义debug格式流录制、流回放，快速复现、定位问题**
- [x] **自带Web UI页面**
- [x] **支持有B帧的视频**


#### ✒ 协议转换支持情况

见： [附录-协议间转换支持情况](https://pengrl.com/lal/#/appendix?id=%e2%9c%92-%e5%8d%8f%e8%ae%ae%e9%97%b4%e8%bd%ac%e6%8d%a2%e6%94%af%e6%8c%81%e6%83%85%e5%86%b5)

### ▦ 二. lalserver 安装、运行

lalserver支持3种安装、运行方式：

#### ✒ 方式1, 源码编译安装

下载源码：

```shell
git clone https://github.com/q191201771/lal.git
```

编译：

```shell
cd lal
export GO111MODULE=on && export GOPROXY=https://goproxy.cn,https://goproxy.io,direct
make
```

> tips:
> 如果使用GoLand等IDE编译，那么编译`lal/app/lalserver`目录即可。
> 如果没有安装Go编译器，可参考[《CentOS或macOS安装GoLang》](https://pengrl.com/p/34426/)，windows操作系统可自行上网搜索教程。

运行：

```shell
./bin/lalserver -c conf/lalserver.conf.json
```

*注意，windows平台将路径分隔符`/`换成`\`*

#### ✒ 方式2, Docker镜像

下载镜像：

```shell
docker pull q191201771/lal
```

运行镜像：

```
$docker run -it -p 1935:1935 -p 8080:8080 -p 4433:4433 -p 5544:5544 -p 8083:8083 -p 8084:8084 -p 30000-30100:30000-30100/udp q191201771/lal /lal/bin/lalserver -c /lal/conf/lalserver.conf.json
```

*如果想构建自己的Docker镜像，可参考 [附录-构建自己的Docker镜像](https://pengrl.com/lal/#/appendix?id=%e2%9c%92-%e6%9e%84%e5%bb%ba%e8%87%aa%e5%b7%b1%e7%9a%84docker%e9%95%9c%e5%83%8f)*

#### ✒ 方式3, 下载编译好的二进制可执行文件

lal提供`linux/macos/windows`平台编译好的lal二进制可执行文件(zip压缩包形式)。
下载地址见： [《github lal最新release版本页面》](https://github.com/q191201771/lal/releases/latest)
下载好的文件按方式1运行，不再赘述。

### ▦ 三. lalserver 使用

#### ✒ 各协议推拉流url地址列表

lalserver启动成功后，就可以开始使用了。
作为流媒体服务，最主要的功能就是流数据转发。比如：

使用ffmpeg推rtmp流：

```shell
ffmpeg -re -i demo.flv -c:a copy -c:v copy -f flv rtmp://127.0.0.1:1935/live/test110
```

使用ffplay拉rtmp流播放：

```shell
ffplay rtmp://127.0.0.1/live/test110
```

更多协议见： [lalserver 各协议推拉流url地址列表](https://pengrl.com/lal/#/streamurllist)

> tips:
> 更多第三方客户端的使用方法见： [常见推拉流客户端信息汇总](https://pengrl.com/lal/#/CommonClient)

#### ✒ lalserver 配置文件

如果你想进一步了解lalserver的功能，可以看看： [lalserver配置文件说明](https://pengrl.com/lal/#/ConfigBrief)

#### ✒ lalserver HTTP事件和接口

通过lalserver提供的丰富的HTTP接口，业务方可以在保持lalserver独立运行的情况下，使用自身熟悉的语言，轻松定制符合自身逻辑的业务系统。

- [lalserver HTTP Notify(Callback/Webhook)事件回调](https://pengrl.com/lal/#/HTTPNotify)
- [lalsrver HTTP API接口](https://pengrl.com/lal/#/HTTPAPI)

#### ✒ lalserver 进阶

- [lalserver 鉴权防盗链](https://pengrl.com/lal/#/auth)
- [lalserver 二次开发 - pub接入自定义流](https://pengrl.com/lal/#/customize_pub)

### ▦ 四. 重新认识lal

#### ✒ lal的三层结构

![lal源码架构图](https://pengrl.com/lal/_media/lal_src_fullview_frame.jpeg?date=211211)

##### ✦ 其他demo

lal项目中，除了`/app/lalserver`这个比较核心的服务之外，在`/app/demo`目录下还额外提供了一些小应用，比如推、拉流客户端，以及压测工具，流分析工具，lalserver集群的调度示例程序等。
这些demo你既可以直接使用，又向你演示了lal所提供的协议栈如何使用。

了解更多请访问： [Demo简介](https://pengrl.com/lal/#/DEMO)

##### ✦ 流媒体协议栈库package/library

客户端和服务端的协议栈都有。
lal中的协议栈都是独立的，分多层设计的。
业务方可以在自身的应用中集成lal的协议栈package库。

##### ✦ Golang通用基础库-naza

lal将非流媒体特有的通用基础库抽象在一个独立的github repo [naza](https://github.com/q191201771/naza)中。

了解更多请访问： [《naza github 地址》](https://github.com/q191201771/naza)： https://github.com/q191201771/naza

#### ✒ lalext

站在巨人的肩膀上才能看的更远。lal在另一个github repo [lalext](https://github.com/q191201771/lalext)中，将lal的代码与第三方的库结合，实现了更丰富的功能。

##### ✦ WebRTC

rtmp转WebRTC的网关

了解更多请访问： [《lalext github 地址》](https://github.com/q191201771/lalext)： https://github.com/q191201771/lalext

##### ✦ SRT

#### ✒ 进行中

- [正在搞的TODO Roadmap-汇总](https://github.com/q191201771/lal/issues/157)
- [不确定啥时候搞的Indefinite delay-汇总](https://github.com/q191201771/lal/issues/37)
- [找人搞的Help wanted-汇总](https://github.com/q191201771/lal/issues/161)

### ▦ 五. 联系作者

- 微信： q191201771
- QQ： 191201771
- 微信群： 加我微信好友后，告诉我拉你进群
- QQ群： 635846365
- 邮箱：191201771@qq.com
- lal github地址： https://github.com/q191201771/lal
- lal官方文档： https://pengrl.com/lal


作者微信，扫码加好友：

![lal作者微信](https://pengrl.com/images/yoko_vx.jpeg?date=220329)

欢迎任何技术和非技术的交流。

