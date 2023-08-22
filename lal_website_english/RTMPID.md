### Chunk stream id

#### Design Idea

According to the design idea of RTMP, it is possible to transmit multiple streams on one link, for example, the most common stream is audio and one stream is video, where the audio and video streams can be turned on and off independently of each other.

Then, if the message in one stream is particularly large (such as the key frame of the video), this may result in the message of the audio stream having to wait for the transmission of this particularly large message before it can be transmitted.

To solve this problem, RTMP chooses to cut the messages that are meaningful to the application layer into *chunks*, so that instead of transmitting one *message* after another, chunks are instead transmitted.  
(Note that the chunks must be complete; you can't send the first half of chunk 1, then chunk 2, then the second half of chunk 1.)

The receiver combines the chunks with the same `chunk stream id` into a complete message and returns it to the application layer.

Here is a simple example:

```
# Assume Video message 1 is large

# In the most primitive way of multistreaming, Audio Message 2 can only wait for Video message 1 to finish transmitting.
| Audio message 1 | Video message 1 | Audio message 2 | Video message2 |

# Cut Video Message 1 into two chunks (for simplicity here, there may be more than two chunks)
# So that Audio Message 2 doesn't need to wait for Video Message 1 to finish being sent.
# In addition, the other messages are now chunks as well
| Audio message 1 | first chunk of Video meesage 1 | Audio Message 2 | last chunk of Video message 2 | Video message 2 |

| last chunk of Video message 2 | Video message 2 | Video message 2 | The following is incorrect, the chunk must remain intact
| first chunk of chunk1 | first chunk of chunk2 | second chunk of chunk1 | second chunk of chunk2 |
```

#### Thinking

So let's think about it, with the common RTMP over TCP scheme today, when the application layer has a message to send, we cut it into chunks, and then send all the cut chunks, and then loop through the next message.

In fact, the chunks of a message are sent one after the other and no other chunks are inserted. On this basis, TCP-based streaming sequentiality cannot achieve the above design of a large message that does not block messages from other streams.

Theoretically, is this RTMP design idea feasible? It is feasible, for example, sending queues not using large FIFO buffers, but in chunks composed of more fine-grained management. Or the use of UDP transmission, generally according to the MTU cut application layer message, basically similar to the concept of chunk.

The above is actually head-of-line blocking versus header blocking, and QUIC HTTP/3 uses a similar idea to deal with this.


#### Practice

Here are some common RTMP projects that use chunk stream ids.

#### obs push stream

```
csid 2 is used to transport SetChunkSize.
csid 3 is used to transport signalling like connect | releaseStream | FCPublish | createStream | publish
csid 4 is used to transfer metaData + audio data + video data.
```

#### ffmpeg push stream

```
csid 2 SetChunkSize
csid 3 connect | releaseStream | FCPublish | createStream
csid 8 publish
csid 4 metaData + video data
csid 6 audio data
csid 3 FCUnpublish | deleteStream
```

Note that when ffmpeg pushes the stream, it sends the connect first, then the SetChunkSize, the receiver should assume the chunk size of the opposite end is 128 when it doesn't receive the SetChunkSize.

#### pulling stream from nginx rtmp module

```
csid 2 Window ACk Size | SetPeerBandwidth | SetChunkSize
csid 3 onBWDone | _result for Connect | _result for createStream
csid 5 onStatus for Play
csid 5 metaData e.g. len 24
csid 2 user control message
csid 5 metaData e.g. len 699
csid 7 video data
csid 6 audio data
```

As you can see, OBS push stream audio and video data share one csid, ffmpeg has two csid, and nginx RTMP module has a separate csid for metaData as well.

How to get the chunk stream id, how to cut the message into chunks, and how to reassemble chunks into messages can be found in the implementation of [lal](https://github.com/q191201771/lal) (https://github.com/q191201771/).

### message stream id

#### OBS push stream

```
msid 0 signalling
msid 1 publish signalling | audio/video data
```

#### ffmpeg push stream

Same as OBS.

#### pull stream from nginx rtmp module

```
msid 0 signalling
msid 1 onStatus of play signalling | audio/video data
```

As you can see, all three projects are using msid 1 to transfer audio and video data.

In lal, `_result for createStream` uses 1 for msid, and publish and play use `_result` to return msid

### transaction id

A field in a message of type `command`.

Although the documentation defines most messages as 0, e.g.:

```
connect tid: 1
onStatus: 0
play: 0
deleteStream: 0
publish: 0
```

However, this is not the case for the specific implementation of each RTMP software, for example:

#### obs push stream

```
connect tid: 1
releaseStream: 2
FCPublish: 3
createStream: 4
publish: 5
FCUnpublish: 6
deleteStream: 7
```

As you can see, it's a self-incrementing process.  
For testing, the server side uses lal, which replies to the message as it is received with the `tid`.

#### ffmpeg push stream

The result is the same as with the OBS push stream.

#### pull stream from nginx rtmp module

```
onBWDone tid: 0
_result of connect: 1
_result of createStream: 2
onStatus of Play: 0
```

The test uses lal on the client, which handles the `tid` in an incremental fashion:

```
connect tid: 1
createStream: 2
play or publish: 3
```

### message type id

This is the simplest one, it indicates what type of message is used in the application layer, e.g. audio (8), video (9), metadata (18), specific signalling (e.g., 1 is SetChunkSize), command type signalling (20), and so on.

### TODO

Nowadays, RTMP's usage scenario is basically one connection corresponding to one push or pull stream.  
In fact, when RTMP is used in Flash, multiple streams can be opened and closed on one connection object, which is the most suitable usage for RTMP semantics. Since I don't have a suitable environment, I didn't test it for the time being.

yoko, 201908

---

Being original is not easy. Reproduction is allowed, but please reference this article as being part of the documentation for the open-source streaming media server [lal](https://github.com/q191201771/lal) (GitHub link).   Official documentation: [https://pengrl.com/lal](https://pengrl.com/lal)
