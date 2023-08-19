rtmp url basics
The format of an rtmp url is similar to an http url. the format is as follows:

    rtmp://<host>[:port]/<appName>/<streamName>[?param1=value1][&param2=value2]

Where rtmp is the scheme field, fixed.

Let's illustrate with rtmp play pull stream (publish is similar).

Logic of rtmp pull stream client:
The first step, if the host part is the domain name, then first through the domain name resolution to get the opposite end ip.
The second step is to use the ip and port to establish a tcp connection. Note that if the port does not exist in the url, 1935 is used.
The third step, the client and the server through rtmp signalling interaction, to establish a good pull channel, and then the server to the client to send the corresponding stream of audio and video data.

Here some students may wonder, the domain name is resolved into an ip for tcp connection, how does the server know what domain name is used by the client to connect?
The answer is that the client through rtmp signalling, the domain name is transmitted to the server. Specifically, it is rtmp connect signalling (note that this connect is not tcp connect) and play signalling.

Look at a simple url example.

Before testing, modify /etc/hosts so that fake.lal.com, the fake test domain, resolves to the local 127.0.0.1 lalserver. The client uses ffmpeg.

ffplay rtmp://fake.lal.com/live/test110
Signalling value:
connect
  appName: live
  tcUrl: rtmp://fake.lal.com:1935/live
play
  streamName: test110
vhost
Let's start with what vhost does.

For example, let's say you have your own rtmp server (which will be referred to as the source) with a lot of streams on it. The source station due to network and load reasons, and can not serve the whole country viewers to pull the stream, so you in order to better play the effect, use Ali CDN to do stream distribution, that is, the user from the edge node of the Ali CDN to pull the stream, Ali CDN from your source station back to the source to pull the stream.
And because you are worried about Ali CDN is not reliable, so you added a Tencent CDN. users can be on any CDN to pull any of the way you source station on the stream.

At this point, the problem is, your source station was pulled flow, how to distinguish which CDN to pull the flow?

Simple practice, you can apply for two domain names, such as Ali using alfake.lal.com, Tencent using txfake.lal.com, their DNS resolution are pointing to the source station IP.

But this has a disadvantage, that is, subsequent changes, the need for frequent operation of the domain name, such as access to CDN3, CDN4...

Then talk about the practice of vhost.

In fact, it is very simple, that is, the url host only one, in the non-host part, fill in a domain name field. Since it's not host, there's no DNS domain name resolution involved. You can name it whatever you want and quickly provide it to the CDN.

Then exactly where to put it, this is actually an industry subterfuge. There are two common:

Also take that previous example rtmp://fake.lal.com/live/test110 to use

In the first one, a directory is added between host and appName. The server needs to resolve it from the rtmp connect signalling.

rtmp://fake.lal.com/alfake.lal.com/live/test110

Signalling value:
connect
  appName: alfake.lal.com/live
  tcUrl: rtmp://fake.lal.com:1935/alfake.lal.com/live
play
  streamName: test110
The second one, which is simpler, is to add a url parameter. The server needs to parse it from the rtmp play signalling.

rtmp://fake.lal.com/alfake.lal.com/live/test110

Signalling value:
connect
  appName: live
  tcUrl: rtmp://fake.lal.com:1935/live
play
  streamName: test110?vhost=alfake.lal.com
The original is not easy to reproduce, please indicate the article from the open source streaming media server lal, Github: https://github.com/q191201771/lal official documentation: https://pengrl.com/lal

yoko, 20210121