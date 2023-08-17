# lal FAQ
The FAQ is displayed at: https://pengrl.com/lal/#/FAQ 
The FAQ documentation is hosted at [lal_website/FAQ.md](https://github.com/lalext/lal_website/FAQ.md) on the GitHub lalext project.

You are welcome to add your own questions, or ask your own questions, or answer the questions at the bottom of the document. 

To participate, you can either submit a PR to modify lal_website/FAQ.md, or follow up in this issue (I'll move the content over periodically).


▌ Recording

▌ H265

▌ RTSP

▌ Application Scenarios

▌ Compilation and operation problems

▌ To be sorted out


**Q: I'm pushing locally OBS on my computer, pulling with VLC, with sound loopback, any suggestions for optimisation?** (202205)
If the audio is picked up from the microphone, it may happen that the sound played out of the stream you pull, may get picked up by the microphone again, and pushed the stream up, looped. This leads to echo, whistling, and other undesirable effects.

You can try instead with a headset.

This is not really a problem for one-way live productions, because the playback side and the push-flow side are not together, and will not loop. For two-way interactive products, you need to do echo cancellation on (at least) your end.

**Q: When using lalext/rtmp2webrtc, the viewing screen jumps? Ghosting?** (202205)
Because Chrome WebRTC uses OpenH264, it only supports the baseline profile, so it doesn't support B-frames, therefore there will be some frames fallback when playing B-frames.

You can check if your RTMP stream has B-frames or not using the following command:
```bash
ffprobe -show_frames -select_streams v -show_entries frame=pict_type -i rtmp://xxxx
```
If the following information is output, it means the RTMP stream has B-frames:
```
[FRAME]
pict_type=B
[/FRAME]
```
Removing B-frames requires re-encoding the video, the command line is as follows.
```bash
ffmpeg -i input.mp4 -codec:v libx264 -bf 0 -codec:a copy -f mp4 input_no_b_frame.mp4
```
**Q: When using lalserver, after pushing the stream, VLC, `ffplay` and other players are stuck at the last frame and won't exit?**
First of all, lalserver has a timeout mechanism, after a period of time, it will disconnect the pull stream player. 
The fact that the player does not exit at the last frame has something to do with the specific implementation of the specific player for the server side after the connection is closed. 
Users can check whether the connection has been closed through lalserver's log, `netstat`, `tcpdump`, and other means. 

**Q: Why don't some protocols use the standard ports in the default configuration? For example, HTTP does not use port 80 and RTSP does not use port 554.**
Because under Linux, binding ports below 1024 requires root privileges, most open source projects will use ports like 8080, 3000 instead of 80 as the default port for testing. 
Superusers can modify the configuration to use the default ports under 1024 .

▌ Recording
**Q: Can recordings be stored by hour? I want each hour to be stored as a separate video file.** (202205)
There are two types of hourly storage:

1. Each video file is 1 hour long. For example, if you start at 12:20, a new video file will be generated at 13:20 and 14:20.
2. Video files are cut so that they all start at the begining of the hour. For example, if you start recording at 12:20, then new video files are generated  at 13:00 and 14:00 respectively.

For the first type, you can use HLS to record and set the length of individual slices to 1 hour.

The second type is slightly more troublesome, lalserver does not directly support it internally, currently there are several recommended ways:

- Use lal's client-side wrapping, recording and other library code to pull streams from lalserver and record by yourself; in this way, the logic of cutting can be freely customised;
- Wait for lalserver to provide HLS recording to generate a new TS file callback. In the callback, decide how to manage the generated TS file. _(TODO)_

Also, if you want lalserver to provide direct support internally, you can go to GitHub and raise an issue, I will consider scheduling support, thanks.

▌ h265
**Q: Does lal support h265 playback?**
lal supports h265. if you want to pull h265 streams from lalserver and play them, you can do that, so long as the player supports it.

**Q: Why does it fail to play h265 FLV/RTMP streams with players like VLC?**
lal supports h265 FLV/RTMP.

But since the official standard of FLV/RTMP doesn't support h265, the native official version of `ffmpeg`, VLC, `flv.js`etc.  don't support playing h265 rtmp/flv directly. 
You can use [thirdparty/build.sh](thirdparty/build.sh) in the lalext project to compile a `ffmpeg` that supports h265. 
Also try: [EasyPlayer.js](https://github.com/tsingsee/EasyPlayer.js/).

▌ RTSP
**Q: Does RTSP support multicast?**
Not at the moment. Will develop later. _(TODO)_

▌ Application Scenarios
**Q: Can lalserver do video conferencing?**
Currently not supported. _(TODO)_

**Q: In the application scenario of capturing IPcamera video, AI processing and then pushing streaming, how does this framework interface with AI processing?**
lal mainly focuses on the transmission aspect, as for how to process the pre-encoding and post-decoding audio and video before and after transmission, lal is not too concerned. 
For example, the stream pushed by IPcamera AI processing described in this question only needs to be encapsulated into a standard protocol format supported by lal, and then it can be used in the interface. 

▌ Compilation, runtime issues
**Q: I don't know how I can run it?**
See https://pengrl.com/#/?id=%e2%96%a6-%e4%ba%8c-lalserver-%e5%ae%89%e8%a3%85%e3%80%81%e8%bf%90%e8%a1%8c

▌ To be sorted out
Q: plugin for lalserver, planning for webrtc
Q: rtsp support RTP over TS and RTP over PS?
Q: encoding format and encapsulation format supported by each protocol
Q: Does rtsp support anti-piracy?
Q: Does rtsp support authentication?
Q: What are the encoding and encapsulation formats supported by each protocol?
Q: Does it support encryption?
Q: Is it free?
Q: Does it support playback?
Q: What is the problem with obs pushing streams to lalserver and how can I troubleshoot it?
Q: What is the latency of lal?
Q: Hikvision surveillance camera, webpage can not play video, phenomenon: VLC can play, but the waiting time is slightly long 5 seconds or so, mpegts.js can not be played at all, watermelon player can not be played. mpegts playback capture packet, showing that it has been in the data pulling, the timeline has been increasing. But it never plays.
(#153)

Q: (#150)
Q: (#126)
