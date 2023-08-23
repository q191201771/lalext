### test.flv

信息如下/The information is as follows:
```
Input #0, flv, from 'test.flv':
  Metadata:
    major_brand     : mp42
    minor_version   : 0
    compatible_brands: isommp42
    encoder         : Lavf57.83.100
  Duration: 00:03:51.42, start: 0.000000, bitrate: 537 kb/s
  Stream #0:0: Video: h264 (High), yuv420p(progressive), 640x360 [SAR 1:1 DAR 16:9], 15 fps, 15 tbr, 1k tbn, 30 tbc
  Stream #0:1: Audio: aac (LC), 44100 Hz, stereo, fltp, 128 kb/s
```

### test.h264

使用如下命令生成的ES流(h264 annexb)/ES stream generated using the following command (H.264 Annex B)

```sh
$ ffmpeg -i test.flv -vcodec copy -an -f rawvideo -vbsf h264_mp4toannexb test.h264
```

信息如下/The information is as follows:

```
Input #0, h264, from 'test.h264':
  Duration: N/A, bitrate: N/A
  Stream #0:0: Video: h264 (High), yuv420p(progressive), 640x360 [SAR 1:1 DAR 16:9], 15 fps, 15 tbr, 1200k tbn, 30 tbc
```
