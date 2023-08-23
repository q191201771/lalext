# lalserver HTTP Notify(Callback/Webhook) event callbacks

As a streaming server, lalserver can pass some internal events to the business side via HTTP POST.  
The business side can better implement their own customised business by combining lalserver's HTTP Notify and [HTTP API Interface](HTTPAPI.md).  

## ▌ I. Related Configuration

HTTP Notify has some configurations in the configuration file as follows (specific to the document [lalserver configuration file description](ConfigBrief.md):

```json
  ...
  "server_id": "1",     // Unique ID of the current lalserver, which can be used to distinguish lalserver nodes when multiple lalserver HTTP Notify callbacks are made to the same address.
  "http_notify": {
    "enable": true,     // Whether to enable HTTP Notify event callbacks.
    "update_interval_sec": 5,   // Interval between update event callbacks, in seconds.
    "on_server_start": "http://127.0.0.1:10101/on_server_start",    // HTTP Notify event callback address for each event
    "on_update": "http://127.0.0.1:10101/on_update",    // Note that this can be set to null for events that you don't care about
    "on_pub_start": "http://127.0.0.1:10101/on_pub_start",    
    "on_pub_stop": "http://127.0.0.1:10101/on_pub_stop",
    "on_sub_start": "http://127.0.0.1:10101/on_sub_start", 
    "on_sub_stop": "http://127.0.0.1:10101/on_sub_stop",
    "on_relay_pull_start": "http://127.0.0.1:10101/on_relay_pull_start",
    "on_relay_pull_stop": "http://127.0.0.1:10101/on_relay_pull_stop",
    "on_rtmp_connect": "http://127.0.0.1:10101/on_rtmp_connect"
  },
...
```

## ▌ II. HTTP Notify List

 1. `on_update`             — Periodic reporting of all groups & sessions.
 2. `on_pub_start`          — Other servers are pushing to the current node.
 3. `on_pub_stop`           — Push stops.
 4. `on_sub_start`          — Other servers are pulling streams from current node.
 5. `on_sub_stop`           — Stopped pulling the stream.
 6. `on_relay_pull_start`   — Pulled stream back to source.
 7. `on_relay_pull_stop`    — Pull back to source stopped.
 8. `on_rtmp_connect`       — Received RTMP connect message.
 9. `on_server_start`       — When the service starts.
10. `on_hls_make_ts`        — When HLS generates each TS slice file.


## ▌ III. Example

All interfaces use the `HTTP POST` method. Here is an example for `on_pub_start`:

```
POST /on_pub_start HTTP/1.1
Host: 127.0.0.1:10101
User-Agent: Go-http-client/1.1
Content-Length: 261
Content-Type: application/json
Accept-Encoding: gzip

{"server_id":"1","protocol":"RTMP","url":"rtmp://127.0.0.1:1935/live/test110?token=1111","app_name":"live","stream_name":"test110","url_param":"token=1111","session_id":"RTMPPUBSUB1","remote_addr":"127.0.0.1:52576","has_in_session":true,"has_out_session":false}
```



## ▌ IV. Interface Details

### 1 `on_update`

Regularly report information about all groups and sessions at intervals determined by the configuration file.

```json
{
    "server_id": "1",                   // Configured in config file.
                                        // This field is used to distinguish between different lalserver nodes 
                                        // when multiple lalservers notify to the same business system address.
    groups: [
      {
        "stream_name": "test110",       // Stream name.
        "app_name": "live",             // Application name.
        "audio_codec": "AAC",           // Audio codec format: "AAC".
        "video_codec": "H264",          // Video codec format: "H264" | "H265".
        "video_width": 640,             // Video width.
        "video_height": 360,            // Video height.
        "pub": { // ----- receives information about the push stream -----
          "session_id": "RTMPPUBSUB1",  // Session ID, a globally unique identifier for the session.
          "protocol": "RTMP",           // Push stream protocol, either: "RTMP" | "RTSP".
          "base_type": "PUB",           // Base type, fixed to "PUB".
          "start_time": "2020-10-11 19:17:41.586",  // Start time of push flow.
          "remote_addr": "127.0.0.1:61353",         // Peer address.
          "read_bytes_sum": 9219247,    // Cumulative read data size (counted from start of push).
          "wrote_bytes_sum": 3500,      // Cumulative sent data size.
          "bitrate_kbits": 436,         // Last 5 seconds bitrate in kbit/s. 
                                        // Equivalent to `read_bitrate_kbits` for pub types unless otherwise declared.
          "read_bitrate_kbits": 436,    // The last 5 seconds of read data bitrate.
          "write_bitrate_kbits": 0      // The last 5 seconds of send data bitrate.
        },
        "subs": [ // Information about ----- pulling streams, multiple protocols may exist, each with multiple possible session connections -----
          {
            "session_id": "FLVSUB1",    // Session ID, globally unique identifier for the session
            "protocol": "FLV",          // Pull streaming protocol, either: "RTMP" | "FLV" | "TS".
            "base_type" "SUB"           // Base type, fixed to "SUB".
            "start_time": "2020-10-11 19:19:21.724",    // Start time of stream pulling.
            "remote_addr": "127.0.0.1:61785",           // Peer address
            "read_bytes_sum": 134,      // Cumulative read data size (calculated from the start of the pull).
            "wrote_bytes_sum": 2944020, // Cumulative size of sent data.
            "bitrate_kbits": 439,       // Last 5 seconds bitrate in kbit/s.
                                        // For sub types, this is equivalent to `write_bitrate_kbits` unless otherwise declared.
            "read_bitrate_kbits": 0,    // Last 5 seconds of read data bitrate.
            "write_bitrate_kbits": 439  // The last 5 seconds of send data bitrate.
          }
        ],
        "pull": { // ----- This node pulls streams from other nodes back to the source -----
          "base_type": "PULL",          // This is fixed to "PULL".
          ...                           // The other fields are the same as the pub's internal fields above, so I won't go into them again.
        },
        "pushs": []                     // Active outbound push information, not provided yet.
      }
    ]
}
```

### 2 `on_pub_start`

Triggered when someone else pushes the stream to the current node. RTMP, RTSP are currently supported, examples are as follows:

1) RTMP
```shell
$ ffmpeg -re -i wontcry.flv -c:a copy -c:v copy -f flv "rtmp://127.0.0.1:1935/live/test110?token=1111"
```
```json
{
    "server_id": "1",                   // id of this lalserver node, same as server_id field in config file.
    "session_id": "RTMPPUBSUB5",        // Session ID, globally unique for the session.
    "protocol": "RTMP",                 // Push streaming protocol, either: "RTMP" | "RTSP".
    "base_type": "PUB",                 // Base type, fixed to "PUB".
    "remotet_addr": "127.0.0.1:52010",  // The address of the other end of the connection.
    "url": "rtmp://127.0.0.1:1935/live/test110?token=1111", // Full URL.
    "app_name": "live",                 // Application name in the RTMP URL.
    "stream_name": "test110",           // Stream name.
    "url_param": "token=1111",          // URL parameters.
    "has_in_session": true,             // Whether or not there is an incoming audio/video stream in the corresponding group.
    "has_out_session": false,           // Whether or not the corresponding group has an outgoing audio/video session.
    "read_bytes_sum": 3407,             // The total number of bytes read since the session was established, in bytes.
    "wrote_bytes_sum": 3500             // The total amount of data in bytes that has been sent since the session was created.
}
```
-----

2) RTSP
```shell
$ ffmpeg -re -i wontcry.flv -c:a copy -c:v copy -f rtsp rtsp://localhost:5544/live/test110
```
```json
{
    "server_id": "1",
    "session_id": "RTSPPUB2",
    "protocol": "RTSP",
    "base_type": "PUB",
    "remotet_addr": "[::1]:57308",
    "url": "rtsp://localhost:5544/live/test110",
    "app_name": "live",
    "stream_name": "test110",
    "url_param": "",
    "has_in_session": true,
    "has_out_session": false,
    "read_bytes_sum": 0,
    "wrote_bytes_sum": 0
}
```

### 3 `on_pub_stop`

Triggered when the push stream stops. Fields are the same as `on_pub_start` and are not repeated here.


### 4 `on_sub_start`

Triggered when someone else pulls the stream from the current node. Currently `RTMP`, `RTSP`, `HTTP-FLV`, `HTTP-TS`, `HLS` are supported; examples are as follows:


1) HTTP-FLV
```shell
$ wget http://127.0.0.1:8080/live/test110.flv\?token\=1111
```
```json
{
    "server_id": "1",
    "session_id": "FLVSUB1",
    "protocol": "FLV",
    "base_type": "SUB",
    "remotet_addr": "127.0.0.1:51889",
    "url": "http://127.0.0.1:8080/live/test110.flv?token=1111",
    "app_name": "live",
    "stream_name": "test110",
    "url_param": "token=1111",
    "has_in_session": false,
    "has_out_session": true,
    "read_bytes_sum": 0,
    "wrote_bytes_sum": 0
}
```
---

2) HTTP-TS
```shell
$ wget http://127.0.0.1:8080/live/test110.ts\?token\=1111
```
```json
{
    "server_id": "1",
    "session_id": "TSSUB1",
    "protocol": "TS",
    "base_type": "SUB",
    "remotet_addr": "127.0.0.1:52170"
    "url": "http://127.0.0.1:8082/live/test110.ts?token=1111",
    "app_name": "live",
    "stream_name": "test110",
    "url_param": "token=1111",
    "has_in_session": false,
    "has_out_session": true,
    "read_bytes_sum": 0,
    "wrote_bytes_sum": 0
}
```

Other protocols such as RTMP and RTSP are similar and will not be repeated here.

Regarding on-demand back-to-source pull streams:

- The business side can trigger the lalserver to pull a stream from a third party via the HTTP API interface `start_relay_pull` when it receives the `on_sub_start` event and `has_in_session` is false (to put it in human terms: someone is coming to look at it but the stream doesn't exist).
- The business side can trigger the lalserver to stop pulling streams from the third party via the HTTP API interface when it receives the `on_sub_stop` event and `has_out_session` is false (in human words: people are gone, no one is watching).


### 5 `on_sub_stop`

Triggered when the pull stream stops. Same fields as `on_sub_start`, no further details.


### 6 `on_relay_pull_start`

Return source pull stream successful.

```json
{
    "server_id": "1",                   // See above.
    "session_id": "RTMPPULL1",          // Session ID, session global unique identifier
    "protocol": "RTMP",                 // Protocol, either: "RTMP" | "RTSP".
    "base_type": "PULL",                // Base type, fixed to "PULL" here.
    "remotet_addr": "127.0.0.1:1935",   // The address of the other end of this connection.
    "url": "rtmp://127.0.0.1:1935/live/test110?token=1111", // full url
    "app_name": "live",                 // Application name in the RTMP URL
    "stream_name": "test110",           // Stream name.
    "url_param": "token=1111",          // URL parameters.
    "has_in_session": true,             // Whether or not there is an incoming audio/video stream in the corresponding group.
    "has_out_session": false,           // Whether or not the corresponding group has an outgoing audio/video stream.
    "read_bytes_sum": 0,                // The total number of bytes read since the session was established, in bytes.
    "wrote_bytes_sum": 0                // The total amount of data sent since the session was created, in bytes.
}
```

### 7 `on_relay_pull_stop`

Triggered when the return source pull stream stops. Fields are the same as `on_relay_pull_stop` and are not repeated here.


### 8 `on_rtmp_connect`

Triggered when the RTMP connect message signalling was received.

```json
{
  "server_id": "1",
  "session_id": "RTMPPUBSUB1",
  "remote_addr": "127.0.0.1:53834",
  "app": "live",
  "flashVer": "FMLE/3.0 (compatible; Lavf57.83.100)",  
  "tcUrl": "rtmp://127.0.0.1:19350/live"
}
```

### 9 `on_server_start`

Triggered when the service starts. Fields are the same as the data field in `/api/stat/lal_info` in the HTTP API.


### 10 `on_hls_make_ts`

Triggered when HLS generates each TS slice file.

```json
{
  "event": "open",          // "open" means that the TS slice file is created.
                            // "close" indicates that the † slice file has been written.
  "stream_name": "test110", // Stream name.
  "cwd":"/Volumes/T7/lal",  // Current working path.
                            // When the HLS directory in the config file is set to a relative path, it's relative to this field.
  "ts_file": "lal_record/hls/test110/test110-1654601694357-2.ts",   // Path on disk for the TS file.
  "live_m3u8_file": "lal_record/hls/test110/playlist.m3u8",         // Path to live m3u8 on disk.
  "record_m3u8_file": "lal_record/hls/test110/record.m3u8",         // Path to recorded m3u8 file on disk.
                                                                    // Note that this field is only meaningful
                                                                    // if recording is enabled.
  "id": 2,                  // ID number of the TS file. Linearly incrementing.
  "duration": 3.333,        // Duration of the TS file in seconds.
                            // Note that this field is only meaningful if `event` is set to "close".
  "server_id": "1"
}
```

NOTE: HttpNotifyVersion = "v0.2.2"
