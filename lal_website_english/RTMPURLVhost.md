# RTMP URL basics

The format of a RTMP URL is similar to a HTTP URL. The format is as follows:

    rtmp://<host>[:port]/<appName>/<streamName>[?param1=value1][&param2=value2]

Where `rtmp` is the scheme field, fixed.

Let's illustrate with RTMP play pull stream (publish is similar).

Logic of RTMP pull stream client:
1. The first step: if the host part is the domain name, then do a domain name resolution to get the opposite end's IP address.
2. The second step is to use the IP and port to establish a TCP connection. Note that if the port does not exist in the URL, 1935 is used (default port).
3. In the third step, the client and the server interact through through RTMP signalling to establish a good pull channel, through which the server sends the client the requested stream of audio and video data.

Here some students may wonder; If the domain name is resolved into an IP for TCP connection, how does the server know what domain name is used by the *client* to connect?  
The answer is that, during the RTMP signalling phase, the client transmits the domain name to the server. Specifically, there is a phase for RTMP connect signalling (note that the connection in not established via TCP connect!) and another for play signalling.

So, let's take a look at a simple URL example.

Before testing, modify `/etc/hosts` so that `fake.lal.com` — a fake test domain — resolves to the local `127.0.0.1` lalserver. The client can then use `ffmpeg` in the following way:
```shell
$ ffplay rtmp://fake.lal.com/live/test110
```
Signalling value:
```yaml
connect
  appName: live
  tcUrl: rtmp://fake.lal.com:1935/live
play
  streamName: test110
```
## vhost

Let's now see what we can achieve with **vhost** instead.

For example, let's say you have your own RTMP server (which will be referred to as **the source**) with a lot of streams on it. The source station, due to network and CPU load reasons, can not serve viewers in the whole country to pull the stream. Therefore, in order to achieve good quality during playback, you can use Ali CDN to do stream distribution, that is, the user pulls its stream from the edge node of the Ali CDN and the Ali CDN calls back to the source to pull the stream.

And because you are worried about Ali CDN not being reliable, you added a Tencent CDN as well. Users can be on any CDN to pull any of the streams on your source station.

At this point, the problem is, when your source station gets a request to pull a stream, how does it know from which CDN to pull it?

A simple solution is to use *two* domain names, one for Ali using alfake.lal.com, and another for Tencent using txfake.lal.com, both of which resolving via DNS to the source station IP address.

But this has a disadvantage, namely, ongoing maintenance and dealing with subsequent changes, such as the potential need for frequent changes to the domain name, or providing access through CDN3, CDN4...

In this scenario, it's better to use a **vhost**.

In fact, this is much simpler. In effect, the URL host just fills in a domain name field in the non-host part. Because it's not in the host part, there's no DNS domain name resolution involved. You can name it whatever you like and quickly make it available to the CDN.

As for where exactly to put it, there are actually a few industry tricks for that. These are the two most common ones.

Take the previous example of `rtmp://fake.lal.com/live/test110`:

The first trick is to add a directory between host and `appName`. The server needs to resolve this during the RTMP connection signalling phase.
```shell
$ ffplay rtmp://fake.lal.com/alfake.lal.com/live/test110
```
Signalling value:
```yaml
connect
  appName: alfake.lal.com/live
  tcUrl: rtmp://fake.lal.com:1935/alfake.lal.com/live
play
  streamName: test110
```  
  
The second trick is even simpler: just add a parameter to the URL. The server needs to parse it during the RTMP play signalling phase.
```shell
$ ffplay rtmp://fake.lal.com/alfake.lal.com/live/test110
```
Signalling value:
```yaml
connect
  appName: live
  tcUrl: rtmp://fake.lal.com:1935/live
play
  streamName: test110?vhost=alfake.lal.com
```
yoko, 20210121

---

Being original is not easy. Reproduction is allowed, but please reference this article as being part of the documentation for the open-source streaming media server [lal](https://github.com/q191201771/lal) (GitHub link).   Official documentation: [https://pengrl.com/lal](https://pengrl.com/lal)