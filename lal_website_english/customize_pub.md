## Secondary development — pub access to custom streams

### ▦ Example Scenario

Let's imagine a scenario where the business side has custom protocol streams in their own system, and now they want to feed them into lalserver, **thus using lalserver's functionality** such as recording, or (using the protocols supported by lalserver) pulling streams from lalserver for playback and so on.  
One current approach is for the business side to convert the stream to a standard protocol stream supported by lalserver in their own system, and push it to lalserver using a network connection.  
Another way is to use the customized pub function provided by lalserver. In that scenario, the business-side code will be executed inside the lalserver process.  
Note that for both methods **the result is the same** after the stream enters lalserver**.  
The difference is only the architecture**. That is, whether the business side code is integrated in lalserver or not.  

In other words, with the first approach, the lalserver is more independent. In the second approach, the lalserver is more tightly integrated with the business side of the system, in some cases reducing some intermediate flows.

### ▦ What

**lalserver** provides an interface at the **code level** where the business side can call lalserver functions to add or remove custom streams to lalserver and input audio/video data.

What does it mean? To put it in human terms, when you start lalserver, there are two ways for the streams to enter the server:

1. Either using a client to push streams to lalserver (lalserver side is usually called *pub*); or
2. Using a configuration file or the HTTP API to notify  lalserver to pull the stream from elsewhere (in this case, lalserver is usually called a *relay pull*).

Now, there is one additional way: on top of lalserver, **write custom code** to pass the stream data (chunks of memory in the same process) *directly* to lalserver.

> This feature is called **customize pub** below and in lalserver.

### ▦ How

Talk is cheap, so let’s go straight to a full example:  
[lal/app/demo/customize_lalserver.go](https://github.com/q191201771/lal/blob/master/app/demo/customize_lalserver/customize_lalserver.go) demonstrates how to use customized pub with the help of reading an H.264 ES stream file and an AAC ES stream file.

Note that in terms of usage scenarios, **lalserver doesn't care about the source of the streams on the business side or the original format of the streams. **

In the business-side code, the stream source can be a file or the network. The raw format can be ES or any other format.  
The business side just needs to **choose the timing according to its own business logic** to call lalserver's interface to create the stream, and **convert the stream to the format required by lalserver** to pass it in.

Other than that, just look at the demo code. The code is very small and well commented.  
The only caveat is that throughout the demo you should think of the code as business-side code. The goal is for you to learn how to interact with the lalserver, not how to use the demo.

Audio and video files for testing can be downloaded from the `avfile` directory of the GitHub repo at https://github.com/q191201771/lalext.

This note updated at 2210 by yoko