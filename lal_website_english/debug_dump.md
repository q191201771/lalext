# Debug dump capture for lal stream debugging

lal supports saving the data dump of the input stream as a binary file, and afterwards, through this dump file, the stream can be fed into lal again for data playback.

If a problem occurs, you can dump the data and provide it to the author of lal to help him analysing it.

Here are the corresponding dump methods for each protocol:

#### ▦ GB28181 

Via the `debug_dump_packet` parameter in the `/api/ctrl/start_rtp_pub` interface of lalserver's HTTP-API

> See specifically: https://pengrl.com/lal/#/HTTPAPI?id=_24-apictrlstart_rtp_pub

#### ▦ RTSP

RTSP has two methods:

##### ✒ lalserver

The first is via the `debug_dump_packet` parameter in the `/api/ctrl/start_relay_pull` interface of the HTTP-API of lalserver

> See specifically: https://pengrl.com/lal/#/HTTPAPI?id=_21-apictrlstart_relay_pull

##### ✒ demo pullrtsp

The second method is to use the demo `app/demo/pullrtsp` to pull the rtsp stream directly

The way to use it is as follows:

```shell
$ ./bin/pullrtsp -i rtsp://localhost:5544/live/test110 -o outpullrtsp.flv -t 0 -d outpullrtsp.laldump
```

The files `outpullrtsp.flv`, `outpullrtsp.laldump`, and `pullrtsp.log` will be generated after pulling the RTSP stream.

#### ▦ customize_pub plugin custom streams

Call the `CustomizePubSessionContext::WithCustomizePubSessionContextOption` method and set the `DebugDumpPacket` parameter in it to the name of the file to save, e.g. `"./dump/test110.laldump"`,  `"/tmp/test110.laldump"`.

yoko, 202212
