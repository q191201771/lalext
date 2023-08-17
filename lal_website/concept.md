> Important Concepts Presented at: https://pengrl.com/lal/#/concept  
> Important Concepts Documentation hosted at [lal_website/concept.md](https://github.com/q191201771/lalext/) on github [lalext project](https://github.com/q191201771/lalext) blob/master/lal_website/concept.md)
>
> Feel free to add your own questions, or ask your own questions, or answer the questions at the bottom of the document.  
>
> Participate by submitting a PR to the document [lal_website/concept.md](https://github.com/q191201771/lalext/blob/master/lal_website/concept.md), or by posting directly to this [issue]( https://github.com/q191201771/lalext/issues/5).

# Important concepts

- Time to first frame (seconds to open)
- Latency
- Lag
- SFU
- Trans encapsulation
- Encoding and decoding
- Pure audio streaming, pure video streaming

#### Time to first frame (seconds to open)

**Time to first frame** refers to the time elapsed from the moment the streaming URL is input in the player, to the rendering and playback of the first video frame.

**Seconds to open** gradually became the agreed name in the industry for the opening time of the first frame. (Of course, it can also be 2 seconds to open, 500 milliseconds to open, etc.).

Another common term for this is _open rate_, which indicates the _percentage_ of viewers who open the first frame within one second. Obviously, the higher the open rate per second, the better.

#### Latency

**Latency** is the difference between the point in time shown on the screen on the playback side, and the point in time when the push stream side produces and pushes this screen frame.

For example, if the anchor of a news show says something at exactly 12:00 noon physical time, and the playback side hears this at 3 seconds past 12:00, then the delay is 3 seconds.

#### Lag

#### SFU

#### Encapsulation

#### encode/decode

#### Pure audio stream, pure video stream

**Pure audio stream** means a stream with only audio and no video. Note that by _no video_, we mean no video data at all, which needs to be distinguished from the case where black screen video data is transmitted.

A **pure video stream** is a stream with only video and no audio. Note that by _no audio_, we mean no audio data at all, which needs to be distinguished from cases such as muted audio data being transferred.

yoko, 202211