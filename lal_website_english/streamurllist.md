# lalserver — List of push and pull stream URL addresses by protocol

| Protocol                  | URL address                                       | Protocol standard port |
|---------------------------|---------------------------------------------------|-----------------------:|
| RTMP push stream          | rtmp://127.0.0.1:1935/live/test110                |                   1935 |
| RTSP push stream          | rtsp://localhost:5544/live/test110                |                    554 |
| RTMP pull stream          | rtmp://127.0.0.1:1935/live/test110                |                   1935 |
| HTTP-FLV pull stream      | http://127.0.0.1:8080/live/test110.flv            |                     80 |
|                           | https://127.0.0.1:4433/live/test110.flv (https address)       |        443 |
| WebSocket-FLV pull stream | ws://127.0.0.1:8080/live/test110.flv              |                     80 |
|                           | wss://127.0.0.1:4433/live/test110.flv (websockets address)    |        443 |
| HLS (m3u8+ts) streaming   | http://127.0.0.1:8080/hls/test110.m3u8 (live address format 1)    |        |
|                           | http://127.0.0.1:8080/hls/test110/playlist.m3u8 (live address format 2) | 80 |
|                           | http://127.0.0.1:8080/hls/test110/record.m3u8 (full recording address) |   |
| RTSP pull stream          | rtsp://localhost:5544/live/test110                |                    554 |
| HTTP-TS pull stream       | http://127.0.0.1:8080/live/test110.ts (http address)      |             80 |
|                           | https://127.0.0.1:4433/live/test110.ts (https address)    |            443 |
|                           | ws://127.0.0.1:8080/live/test110.ts (websocket address)   |             80 |
|                           | wss://127.0.0.1:4433/live/test110.ts (websockets address) |            443 |

## About ports

**Tips:** The ports in the example table are the default values in the lal configuration file, and can be modified by the user as described in the configuration file.

If you use the protocol standard port, the port in the address can be omitted, for example, the default port of HTTP is 80, then change http://127.0.0.1:80/live/test110.flv into http://127.0.0.1/live/test110.flv.

If you are not familiar with the push and pull flow client how to work with the use, you can refer to the [Common push and pull flow client information summary](CommonClient.md).
