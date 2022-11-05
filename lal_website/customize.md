# lalserver 二次开发

### ▦ lal的正确使用姿势

从入门到通(fang)关(qi)：

| 编号   | 方式                                                 | 可满足业务方多少功能需求 | 相关文档                                                                                                                           |
|------|----------------------------------------------------|--------------|--------------------------------------------------------------------------------------------------------------------------------|
| 1    | 直接启动lalserver                                      | 60%          | [lalserver 安装、运行](https://pengrl.com/lal/#/?id=%e2%96%a6-%e4%ba%8c-lalserver-%e5%ae%89%e8%a3%85%e3%80%81%e8%bf%90%e8%a1%8c)    |
| 2    | 改改lalserver配置文件                                    | 70%          | [lalserver 配置文件说明](https://pengrl.com/lal/#/ConfigBrief)                                                                       |
| 3    | 业务方服务与lalserver的HTTP Notify(回调事件)，HTTP API(接口)配合   | 80%          | [lalserver HTTP Notify事件回调](https://pengrl.com/lal/#/HTTPNotify) <br> [lalserver HTTP API接口](https://pengrl.com/lal/#/HTTPAPI) |
| 4    | 与lal中的其他demo配合                                     | 85%          | [Demo 介绍](https://pengrl.com/lal/#/DEMO)                                                                                       |
| 5    | 与lalext中的扩展配合                                      | 90%          | [lalext](https://github.com/q191201771/lalext)                                                                                 |
| 6    | 基于lalserver做二次开发                                   | 95%          | [lalserver 二次开发](https://github.com/q191201771/customize)                                                                      |
| 7    | 丢掉lalserver，基于lal库做自己的应用                           | 99%          | nil                                                                                                                            |
| 8    | 丢掉lal                                              | 100%         | nil                                                                                                                            |

越上面，越易用，越下面，越灵活。  

本文聚焦第6关副本——基于lalserver做二次开发。

### ▦ lalserver 二次开发

#### ✒ lal项目中的lalserver是什么？

从功能角度看，lalserver是lal项目中的一个**开箱即用**的流媒体服务器应用。它最基础的功能是接收**标准协议**流（如rtmp、rtsp等）作为输入，完成协议转换、转发、录制等等一系列功能。  
从代码模型（程序、进程）角度，lalserver是一个包含了main函数的、编译出来就是exe的、直接可以运行的程序。  

> main方法的代码见 https://github.com/q191201771/lal/blob/master/app/lalserver/main.go

#### ✒ 如何集成lalserver？

有的业务方，想集成lalserver，也即将lalserver的功能作为自身exe的一部分，这是可行的。  
事实上，如果你打开上面的main.go查看，会发现它只有短短20行代码，就干了两件简单的事：

1. 读取命令行参数
2. 创建初始化一个ILalServer对象，并调用它的RunLoop方法运行

相关代码如下：

```
	lals := logic.NewLalServer(func(option *logic.Option) {
		option.ConfFilename = confFilename
	})
	err := lals.RunLoop()

// 代码基于v0.31.1版本，代码位置： https://github.com/q191201771/lal/blob/master/app/lalserver/main.go#L27
```

因此，业务方要集成lalserver，只需要在自己的代码中创建ILalServer并使用即可。

#### ✒ 什么是lalserver 二次开发？

根据上文，你大致可以把lalserver应用看成是对ILalServer对象的一个套壳，这么说也没啥毛病。下面我们再展开说说。

lal项目中，package库可以粗略的分为两大部分：

1. 协议库，它是逻辑无关的、纯粹的，只包含标准协议栈、网络IO等。  
2. lalserver专有的库，叫pkg/logic，它包含的是lalserver的专有逻辑。  

lalserver二次开发，就是既使用lalserver的logic库的基础功能，又不完全使用，自己还搞点定制化、插件化的开发。  
你可以理解为，logic内提供了很多默认的实现，比如已接入了一些标准协议，流管理、流转发、鉴权、配置管理等等，  
如果满足不了你的需求，比如你想接入自定义的协议，自定义鉴权等等，你可以针对性的做二次开发。  

lalserver二次开发的能力边界，取决于logic提供的接口。我们的目标是：

1. 凯撒的归凯撒，上帝的归上帝。业务方不要魔改lal pkg库中的代码
   1. 通用的，尽量提PR，为开源做贡献
   2. 业务方特有的逻辑，可以通过插件注册扩展
2. 封装易用性。业务方可以尽量少的关心细节，做到面向接口开发

#### ✒ 如何二次开发？

上文`如何集成lalserver`中的代码(`app/lalserver/main.go`)演示了LalServer对象的最简单用法，接下来，我进一步讲解扩展ILalServer的两种手段：

第一种手段，是初始化时的配置。

配置项相关代码如下：

```
type Option struct {
	// ConfFilename 配置文件，注意，如果为空，内部会尝试从 DefaultConfFilenameList 读取默认配置文件
	//
	ConfFilename string

	// NotifyHandler
	//
	// 事件监听
	// 业务方可实现 INotifyHandler 接口并传入从而获取到对应的事件通知。
	// 如果不填写保持默认值nil，内部默认走http notify的逻辑（当然，还需要在配置文件中开启http notify功能）。
	// 注意，如果业务方实现了自己的事件监听，则lal server内部不再走http notify的逻辑（也即二选一）。
	//
	NotifyHandler INotifyHandler

	// TODO(chef): [refactor] 考虑用INotifyHandler实现ModConfigGroupCreator和IAuthentication 202209

	// ModConfigGroupCreator
	// This func help us modify the group configuration base on appName or streamName
	// so that group can have it own configuration (configuration can be in other source like db)
	// It will help us reduce resource usage if we just want some specific group record flv or hls...
	ModConfigGroupCreator ModConfigGroupCreator

	// Authentication
	// This interface make authenticate customizable so that we can implement any authenticate strategy like jwt...
	Authentication IAuthentication
}

// 代码基于v0.31.1版本，代码位置： https://github.com/q191201771/lal/blob/master/pkg/logic/logic.go#L83
```

第二种手段，是LalServer对象上的提供的方法。

ILalServer接口定义如下：

```
type ILalServer interface {
	RunLoop() error
	Dispose()

	// AddCustomizePubSession 定制化增强功能。业务方可以将自己的流输入到 ILalServer 中
	//
	// @example 示例见 lal/app/demo/customize_lalserver
	//
	// @doc 文档见 <lalserver二次开发 - pub接入自定义流> https://pengrl.com/lal/#/customize_pub
	//
	AddCustomizePubSession(streamName string) (ICustomizePubSessionContext, error)

	// DelCustomizePubSession 将 ICustomizePubSessionContext 从 ILalServer 中删除
	//
	DelCustomizePubSession(ICustomizePubSessionContext)

	// StatLalInfo StatAllGroup StatGroup CtrlStartPull CtrlKickOutSession
	//
	// 一些获取状态、发送控制命令的API。
	// 目的是方便业务方在不修改logic包内代码的前提下，在外层实现一些特定逻辑的定制化开发。
	//
	StatLalInfo() base.LalInfo
	StatAllGroup() (sgs []base.StatGroup)
	StatGroup(streamName string) *base.StatGroup
	CtrlStartRelayPull(info base.ApiCtrlStartRelayPullReq) base.ApiCtrlStartRelayPull
	CtrlStopRelayPull(streamName string) base.ApiCtrlStopRelayPull
	CtrlKickSession(info base.ApiCtrlKickSessionReq) base.HttpResponseBasic
}

// 代码基于v0.31.1版本，代码位置： https://github.com/q191201771/lal/blob/master/pkg/logic/logic.go#L18
```

特定功能二次开发的具体文档：

- [二次开发-pub接入自定义流](https://pengrl.com/lal/#/customize_pub)

有不明白的地方联系我，我会进一步丰富文档。

this note updated at 2210 by yoko
