# lalserver Automatic overlay of muted audio

### ▦ Background

Without sound, the best playing can't be heard. (`</sarcasm>`)

Many players (especially some Web JS web players) may fail to play pure video streams (i.e. those without audio), have exceptionally slow opening times for the first frame, and have large delays.  

The meanings of **pure video stream**, **first frame open time**, **latency** can be found here: [Concepts](concept.md)

There are two reasons for the absence of audio within the stream:

- There is no audio in the source stream (i.e. the stream that is pushed to the lalserver).
- There is audio in the source stream, but when it is converted to other encapsulation protocols, the other encapsulation format does not support the audio encoding format (e.g. if you need to convert to FLV encapsulation protocol for web browser playback, the audio encoding format of the input stream is Opus, and FLV does not support Opus).

The above two scenarios are more common on cameras with RTSP protocol, and I've had a lot of feedback from the community.

### ▦ Features

To solve the above problems, lalserver supports automatically detecting whether there is audio in the stream or not, and adding muted audio data to it when no audio is detected.  
The principle is simple and direct, you can't do it without audio, so I've generated some for you, muted audio is also audio, don't take beanbag as dry food (_T. N.: Chinese saying_).

### ▦ How it works

The corresponding configuration item:

```json
// Configuration for all input-based streams (including all protocols, all input types)
// For details see: https://pengrl.com/lal/#/dummy_audio
"in_session": {
  // Whether to enable dynamic detection of added mute AAC data.
  // If enabled, all input-type streams that have exceeded the `add_dummy_audio_wait_audio_ms` time and still have not
  // receive audio data, then AAC data will be automatically overlayed for this stream
  "add_dummy_audio_enable": false,
  // In milliseconds, see `add_dummy_audio_enable`.
  "add_dummy_audio_wait_audio_ms": 150
},
```

> Hint: the above is sourced from the [lalserver configuration file description](ConfigBrief.md), open the source document for a more friendly reading format.

### ▦ Impact

(Turning on automatic overlay of muted audio) may also have an adverse effect — increasing the first frame open time.  
What the hell, as much good as bad?  
Let me explain, it starts with the implementation...

(Turn on the automatic overlay mute audio feature) lalserver in the initial reception of the stream data, there is an analysis phase, to analyse the stream for the presence of audio.  
There are two triggers for this phase to exit, either audio is received or the `add_dummy_audio_wait_audio_ms` threshold time is reached.  

Note some implementation details (haha):

1. lalserver will cache the video data during the analysis phase.
2. When exiting the analysis phase, the cached video will be sent instantaneously.
3. After exiting the analysis phase, lalserver resumes normal forwarding (i.e., it sends the data as soon as it receives it, but of course, if it needs to add muted audio, it adds it).

Therefore:

- For audio streams, there is no great impact, when to receive the audio when normal, generally is 100 milliseconds up and down;
- For streams without audio, the previous caching phase depends on the `add_dummy_audio_wait_audio_ms` threshold time.

There is no need to configure `add_dummy_audio_wait_audio_ms` too large, generally the interval between audio frames is a few tens of milliseconds, and this can be set to less than a second.

It's worth mentioning that if you don't turn on the auto overlay mute audio feature, then it won't have any effect on the stream.

#### Personal advice:

If you have the problem described in the background above, try turning this feature on. If you know definitively that the playback side (or your downstream pull streaming side) can handle streams without audio just fine, then there's no need to turn it on.

**TODO** Why does the player behaves this way? Don't ask why it does this, asking means I don't do player development, I don't know, and I'm afraid to ask.

End of this article. Have a nice day!

yoko, 202211
