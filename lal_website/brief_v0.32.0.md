# LAL v0.32.0 released: better support for pure video streaming

The Go language streaming open source project [LAL](https://github.com/q191201771/lal) released v0.32.0 today. Exactly one month from the last version, [LAL](https://github.com/q191201771/lal) still maintains the state of efficient iteration.

> LAL project address: https://github.com/q191201771/lal

I'd like to pick three important changes and briefly introduce them:

#### ▌I, Automatic overlay of muted audio

This is to better support [pure video streaming](https://pengrl.com/lal/#/concept?id=%e7%ba%af%e9%9f%b3%e9%a2%91%e6%b5%81%ef%bc%8c%e7%ba%af%e8%a7%86%e9%a2%91%e6% b5%81), to solve the problem that the first frame opening time may be particularly slow and delayed when many players play live streams of pure video.

For details, I've written a separate document: [lalserver auto overlay mute audio](https://pengrl.com/lal/#/dummy_audio)

#### ▌II. Support for RTMPS and RTSPS

Encryption of RTMP and RTSP protocols based on TLS/SSL brings higher security.

RTMPS and RTSPS can be enabled through the configuration file of [lalserver](https://github.com/q191201771/lal). Here is an example for RTMPS:

```go
  "rtmp": {
    // Whether to enable listening for the rtmp service.
    // Note that the enable switch in the config file for each protocol type should be turned on as needed to avoid unnecessary protocol conversion overhead.
    "enable": true,

    // The port on which the RTMP service listens, and where clients push and pull streams to and from the lalserver.
    "addr": ":1935",

    // Whether or not to enable the RTMPS service.
    // Note that RTMP and RTMPS can have either one or all of them turned on or off.
    "rtmps_enable": true,

    // Port address on which the RTMPS service listens.
    "rtmps_addr": ":4935",

    // Local cert file for RTMPS.
    "rtmps_cert_file": "./conf/cert.pem",

    // Local key file for RTMPS.
    "rtmps_key_file": "./conf/key.pem",
    ...
  },
```

> Hint: the above is sourced from ["lalserver configuration file description"](https://pengrl.com/lal/#/ConfigBrief), open the source document for a more friendly reading format.

#### ▌III, Better support for more RTSP cameras

For example:

- RTP stack support parsing padding and CSRC in header
- Fix the problem that AAC RTP type is not a standard value which leads to failure to merge frames. Improved compatibility.
- Fix a bug which made RTSP auth fail sometimes.

All of the above enhancements were made in response to real community feedback.

#### ▌IV. More changes

There are a few more modifications that I won't introduce one by one, they are roughly as follows:

> - [feat] Demo: pullhttpflv pullhttp-flv can be stored as flv file.
>
> - [opt] Secondary: after DelCustomizePubSession, calling FeedAvPacket method of the deleted object will return an error.
>
> - [opt] Secondary Development: Support initialising ILalServer directly using JSON string as configuration content.
>
> - [opt] Compatibility optimisation. Use pre-adjustment timestamp if timestamp adjustment fails when transferring ts.
>
> - [opt] Compatibility optimisation. When rtmps and rtsps fail to load signature files, only print logs without exiting lalserver.
>
> - [fix] http-api: fix bug where sub http-flv remote_addr field had no value.
>
> - [log] Print RTSP signalling. Enriched several error logs, e.g. transferring its exceptions.
>
> - [doc] New document: important concepts https://pengrl.com/lal/#/concept
>
> - [doc] Added documentation: important concepts
>
>   
>
>   The above is extracted from ["lal CHANGELOG Release Log"](https://pengrl.com/lal/#/CHANGELOG), you can get more details from the source document.

#### Learn more about lal

- [github](https://github.com/q191201771/lal)
- [Official documentation](https://pengrl.com/lal)
- [Contact Author](https://pengrl.com/lal/#/Author)

End of this article. Have a nice day! 

yoko, 202211