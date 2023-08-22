# Let's start with the conclusion!

To play a RTSP stream splash screen, you can try the following two simple methods first:

1. Use interleaved (RTP over TCP) mode to transfer data; *or*
2. Increase the socket receive buffer (especially if the objective conditions of the network are not too bad, such as a local loop, or intranet).

The corresponding command line parameters for `ffplay`/`ffmpeg` are, respectively:

1. `rtsp_transport tcp`
2. `buffer_size 1000000`

Note that this article does not deal with UDP transmission, NACK, FEC, bit rate adaptive content, etc. That is another level of complexity, we will talk about it on a future article!

## Rationale

A user has raised a bug with lal, see: https://github.com/q191201771/lal/issues/46

Simply put, when using lalserver as a stream forwarding server, `ffplay` pulls RTSP stream playback with a splash screen.

## Testing & gathering information

1. Push large bitrate RTSP stream (~3Mb/s) to lalserver, `ffplay` plays RTSP stream with splash screen.
2. Push large bitrate RTSP stream to lalserver, use `ffplay` to play RTMP stream normally.
3. Push large bitrate RTSP stream to lalserver, use `ffplay` to play RTSP (RTP over TCP) stream normally.
4. Push large bitrate RTSP stream to lalserver, use VLC to play RTSP streams.
5. Push large bitrate RTSP stream to lalserver, use `pullrtsp` (a widget in lal) to pull RTSP stream and store it as FLV file; after that, play the file normally.
6. Push small bitrate RTSP stream to lalserver, use `ffplay` to play the RTSP stream, it works.
7.Push large bitrate RTSP stream to other RTSP servers, use `ffplay` to play RTSP streams.

From point 1 and 2, we can judge that the part of lalserver that *receives* RTSP push streams is not problematic, that is, the uplink is not problematic.  
From points 3 and 4, we can judge that the part of lalserver *sending* RTSP streams is normal for some of the stream pulling clients.  
Combined with points 5 and 6, it can be roughly judged that the problem may be related to `ffplay` or the test environment.

Then the core problem is still to see *why* `ffplay` will show a splash screen!

## Analysing ffplay

Looking at `ffplay`'s logs, I found that the splash screen is always accompanied by the following warning logs:

    [rtsp @ 0x7f93a8804400] max delay reached. need to consume packet
    [rtsp @ 0x7f93a8804400] RTP: missed 11 packets

I went to the `ffmpeg` source code, found the source code corresponding to the log, and read it briefly.

The context logic of the first line of the log is that reading RTP UDP data has timed out and you need to force consumption of the cache queue.  
The second line of the log means that the sequence numbers of the packets in the cache queue are not consecutive.

Ok, the cause of the splash screen was found, let's see how to solve.

First of all, I read the timeout related logic and found the variable `max_delay`.  
Then I continued to look for associated variables, thus finding the variable `reorder_queue_size`, corresponding to the set number of packets to buffer for handling of reordered packets.  
Then I found the variable `buffer_size`, which corresponds to the underlying protocol send/receive buffer size.

All three of these variables can be set via command line arguments.  
The first two are related to higher-level logic, and the third one sets the socket receive buffer (a kernel-level parameter).

Which one to change?

According to my personal experience, I usually try to increase the `buffer_size` first. After all, it's best to receive the data completely first; if it's not received in a timely fashion, then we go to the next step in the processing.

```shell
$ ffplay -buf fer_size 1000000 rtsp://127.0.0.1:5544/live/test110
```

No more splash screen in my environment, problem solved!

## Follow-up

Note that if `ffplay` sets `buffer_size`, the following message appears in the log:

    Attempted to set receive buffer to size 1000000 but it only ended up set as ...

It means that setting the socket receive buffer failed. If you're on a Linux system, you can try to use the following command line commands before starting `ffplay` to modify the default value of the socket receive buffer and the maximum value allowed to be set.

The method is as follows:

```shell
$ echo 2000000 > /proc/sys/net/core/rmem_default
$ echo 2000000 > /proc/sys/net/core/rmem_max
```

yoko, 20210206

---

Being original is not easy. Reproduction is allowed, but please reference this article as being part of the documentation for the open-source streaming media server [lal](https://github.com/q191201771/lal) (GitHub link).   Official documentation: [https://pengrl.com/lal](https://pengrl.com/lal)