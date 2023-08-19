Let's put the conclusion first:

To play rtsp stream splash screen, you can try the following two simple methods first:

Use interleaved (rtp over tcp) mode to transfer data
Increase the socket receive buffer (especially if the objective conditions of the network are not too bad, such as a local loop, or intranet)
The corresponding command line parameters for ffplay/ffmpeg are respectively:

- rtsp_transport tcp
- buffer_size 1000000

Note that this article does not deal with UDP transmission, NACK, FEC, bit rate adaptive content, that is another level of things, we will talk about it later!

Reason

A user has raised a bug with lal, see: https://github.com/q191201771/lal/issues/46

Simply put, when using lalserver as a stream forwarding server, ffplay pulls rtsp stream playback with a splash screen.

Testing, gathering information

Push large bitrate rtsp stream (~3Mb/s) to lalserver, ffplay plays rtsp stream with splash screen.
Push large bitrate rtsp stream to lalserver, use ffplay to play rtmp stream normally.
Push large bitrate rtsp stream to lalserver, use ffplay to play rtsp (rtp over tcp) stream normally.
Push large bitrate rtsp stream to lalserver, use vlc to play rtsp streams.
Push large bitrate rtsp stream to lalserver, use pullrtsp (a widget in lal) to pull rtsp stream and store it as flv file, after that, play the file normally.
Push small bitrate rtsp stream to lalserver, use ffplay to play the rtsp stream, it works.
Push large bitrate rtsp streams to other rtsp servers, use ffplay to play rtsp streams.
From point 1 and 2, we can judge that the part of lalserver that receives rtsp push streams is not problematic, that is, the uplink is not problematic.
From points 3 and 4, we can judge that the part of lalserver sending rtsp streams is normal for some of the stream pulling clients.
Combined with points 5 and 6, it can be roughly judged that the problem may be related to ffplay or the test environment.

Then the core problem is still to see why ffplay will be splash screen.

Analysing ffplay

Looking at ffplay's logs, I found that the splash screen is always accompanied by the following warning logs:

[rtsp @ 0x7f93a8804400] max delay reached. need to consume packet
[rtsp @ 0x7f93a8804400] RTP: missed 11 packets
Go to the ffmpeg source code, find the source code corresponding to the log, and read it briefly.

The context logic of the first line of the log is that reading rtp udp data has timed out and you need to force consumption of the cache queue.
The second line of the log means that the seq numbers of the packets in the cache queue are not consecutive.

ok, the cause of the screen is found, let's see how to solve.

First of all, read the timeout related logic, found the variable max_delay.
Then continue to look for associated, found the variable reorder_queue_size, corresponding to the meaning of set number of packets to buffer for handling of reordered packets.
Then we found the variable buffer_size, which corresponds to the meaning of Underlying protocol send/receive buffer size.

All three of these variables can be set via command line arguments.
The first two are related to higher-level logic, and the third one sets the socket receive buffer (a kernel-level parameter).
Which one to change?

According to my personal experience, I try to increase the buffer_size first. After all, as soon as possible, complete receipt of data is the best, not timely receipt, receipt is not complete is the next step in the processing.

$ffplay -buffer_size 1000000 rtsp://127.0.0.1:5544/live/test110
No more splash screen in my environment, problem solved.

Follow-up

Note that if ffplay sets buffer_size the log appears:

Attempted to set receive buffer to size 1000000 but it only ended up set as ...
It means that setting the socket receive buffer failed. If it is a linux system, you can try to use the following command line commands before starting ffplay to modify the default value of the socket receive buffer and the maximum value allowed to be set.

The method is as follows:

echo 2000000 > /proc/sys/net/core/rmem_default
echo 2000000 > /proc/sys/net/core/rmem_max
The original is not easy to reproduce, please indicate the article from the open source streaming media server lal, Github: https://github.com/q191201771/lal official documentation: https://pengrl.com/lal

yoko, 20210206