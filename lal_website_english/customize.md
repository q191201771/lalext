# lalserver secondary development

### ▦ lal ecosystem

From the introductory level to expert:

| Level |         Mode/style                                                     | Functionality (business side) | Relevant documentation                                                                                                          |
|---|------------------------------------------------------------------------|-----:|------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 1 | Start the lalserver directly                                           |  60% | [lalserver Installation and Operation](https://pengrl.com/lal/#/?id=%e2%96%a6-%e4%ba%8c-lalserver-%e5%ae%89%e8%a3%85%e3%80%81%e8%bf%90%e8%a1%8c)   |
| 2 | Changing the lalserver configuration file                              |  70% | [lalserver Configuration File Description](https://pengrl.com/lal/#/ConfigBrief)                                                                     |
| 3 | Business-side services work with lalserver's HTTP Notify/HTTP API.     |  80% | [lalserver HTTP Notify event callbacks](https://pengrl.com/lal/#/HTTPNotify) \|  [lalserver HTTP API connection](https://pengrl.com/lal/#/HTTPAPI) |
| 4 | Work with other demos in lal                                           |  85% | [Introduction to the demos](https://pengrl.com/lal/#/DEMO)                                                                                        |
| 5 | Work with lalext extensions                                            |  90% | [lalext](https://github.com/q191201771/lalext)                                                                                 |
| 6 | Secondary development based on lalserver                               |  95% | [lalserver secondary development](https://pengrl.com/lal/#/customize)                                                                                   |
| 7 | Ditch lalserver and make your own application built on the lal library |  99% | nil                                                                                                                                                        |
| 8 | Drop lal completely                                                    | 100% | nil                                                                                                                                                        |

The higher the level, the easier it is to use, and the lower down, the more flexible it is. 

This article focuses on level 6  — doing secondary development based on lalserver.

### ▦ lalserver secondary development

#### ✒ How does lalserver fit into the lal project?

From a functional point of view, lalserver is an **out-of-the-box** streaming server application in the overall lal project. Its most basic function is to receive **standard protocol** streams (e.g. RTMP, RTSP, etc.) as input, and complete a series of functions such as protocol conversion, forwarding, recording, and so on.  
From the code model (programme, process) point of view, lalserver is a directly runnable programme that contains the main function and compiles as an executable binary.  

The code for the main method is available at https://github.com/q191201771/lal/blob/master/app/lalserver/main.go.

#### ✒ How to integrate lalserver with code from third parties?

There are business parties that want to integrate lalserver, i.e. make the lalserver functionality part of their own applications, which is perfectly possible.  
In fact, if you open the `main.go` file above and check it out, you'll see that it's just 20 lines of code that does two simple things:

1. read the command line arguments
2. create and initialise an `ILalServer` object and call its `RunLoop` method to run it.

The relevant code is as follows:

```go
	lals := logic.NewLalServer(func(option *logic.Option) {
		option.ConfFilename = confFilename
	})
	err := lals.RunLoop()

// Code based on version v0.31.1, code location: https://github.com/q191201771/lal/blob/master/app/lalserver/main.go#L27
```

So, for the business side to integrate lalserver, you just need to create ILalServer in your own code and use it.

#### ✒ What is lalserver secondary development?

Based on the above, you can roughly think of a lalserver application as a shell for the ILalServer object, and there's nothing wrong with saying that. Let's expand on that.

In the lal project, the package library can be roughly divided into two main parts:

1. protocol libraries, which are logic-independent and pure Go, and contain only standard protocol stacks, network I/O, etc.  
2. lalserver proprietary library, called pkg/logic, which contains lalserver's proprietary logic.  

lalserver secondary development is to use lalserver's logic from the library of  basic functions, but also engage in a little customisation and plug-in development.  
You can understand that logic provides a lot of the default implementation, such as: access to a number of standard protocols, flow management, flow forwarding, authentication, configuration management, and so on.  
If the basic library can't meet your needs — for example, you want to access custom protocols, custom authentication and so on — you can target the secondary development.  

The boundary of lalserver secondary development capability depends on the interface provided by logic. Our goal is:

1. What is Caesar's to Caesar, God's to God. The business side should not modify the code in the lal pkg library.
   1. Generic code, try to mention PR, contribute to open source;
   2. Business-side-specific logic, can be extended through plug-in registration.
2. Encapsulate ease of use. The business side can care as little as possible about the details to do interface-oriented development.

#### ✒ How to develop *twice*?

The code in `How to Integrate lalserver` above (`app/lalserver/main.go`) demonstrates the simplest usage of the LalServer object. Next, I further explain the two ways of extending `ILalServer`:

The first method is to change the configuration during initialisation.

The code associated with the configuration items is as follows:

```go
type Option struct {
	// ConfFilename configuration file. Note that if it's empty, internally it will try to read the default configuration file from DefaultConfFilenameList.
	ConfFilename string

	// NotifyHandler event listener.
	// The business side can implement the INotifyHandler interface and pass it in to get notified of events.
	// If not filled in, the default value is nil, and http notify is used internally (of course, http notify needs to be enabled in the config file).
	// Note that if the business side implements its own event listener, the lal server no longer uses HTTP notify logic (i.e., it's an either/or).
	//
	NotifyHandler INotifyHandler

	// TODO(yoko): [refactor] Consider implementing ModConfigGroupCreator and IAuthentication 202209 with INotifyHandler.

	// ModConfigGroupCreator.
	// This func helps us modify the group configuration base on appName or streamName // so that group can have it owned.
	// so that group can have it's own configuration (configuration can be in other source like db)
	// It will help us reduce resource usage if we just want some specific group record flv or hls...
	ModConfigGroupCreator IModConfigGroupCreator

	// Authentication.
	// This interface makes authentication customisable so that we can implement any authentication strategy such as JWT...
	Authentication IAuthentication
}

// Code based on version v0.31.1, code location： https://github.com/q191201771/lal/blob/master/pkg/logic/logic.go#L83
```

The second way is to use the methods provided by the `ILalServer` object.

The `ILalServer` interface is defined as follows:
```go
type ILalServer interface {
	RunLoop() error
	Dispose()

	// AddCustomizePubSession customisation enhancements. The business side can feed their own streams into the ILalServer
	//
	// @example See lal/app/demo/customize_lalserver for an example.
	// @example
	// @doc documentation is available at «lalserver secondary development - pub access to custom streams» https://pengrl.com/lal/#/customize_pub
	// 
	AddCustomizePubSession(streamName string) (ICustomizePubSessionContext, error)

	// DelCustomizePubSession removes ICustomizePubSessionContext from ILalServer.
	//
	DelCustomizePubSession(ICustomizePubSessionContext)

	// StatLalInfo StatAllGroup StatGroup CtrlStartPull CtrlKickOutSession
	// — Some APIs for getting state and sending control commands.
	// The purpose is to allow the business side to implement some custom logic in the outer layer without modifying the code in the logic package.
	//
	StatLalInfo() base.LalInfo
	StatAllGroup() (sgs []base.StatGroup)
	StatGroup(streamName string) *base.StatGroup
	CtrlStartRelayPull(info base.ApiCtrlStartRelayPullReq) base.ApiCtrlStartRelayPull
	CtrlStopRelayPull(streamName string) base.ApiCtrlStopRelayPull
	CtrlKickSession(info base.ApiCtrlKickSessionReq) base.HttpResponseBasic
}

// Code based on version v0.31.1, code location: https://github.com/q191201771/lal/blob/master/pkg/logic/logic.go#L18
```

Documentation for secondary development of specific features:

- [secondary-development-pub-access-custom-streams](https://pengrl.com/lal/#/customize_pub)

Contact me if you don't understand anything, I will further update the documentation.

This note updated on 2210 by yoko
