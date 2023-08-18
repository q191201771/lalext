# lalserver configuration file description

The lalserver configuration file is in JSON format.  

In order to make it easier to read and find, this document uses two forms of describing the configuration file:

- The annotated version keeps the original format of the JSON file and adds comments at the end of each line.
- The descriptive version uses the first level of sub-items as a separate header, and the internal block contains its sub-items.


## Annotated version

```json
{
  "# doc of config": "https://pengrl.com/lal/#/ConfigBrief",    // Link to the documentation for the config file. Not used by the application.
  "conf_version": "0.4.1",                  // Configuration file version number, should not be changed manually by the business side.
                                            // The version is checked by the application to see if it is the same as the one declared in the code.
  "rtmp": {
    "enable": true,                         // Whether to enable listening to the RTMP service.
                                            // Note that the enable switch in the configuration file for each protocol type should be turned on only as
                                            // needed to avoid unnecessary protocol conversion overhead.
    "addr": ":1935",                        // The port on which the RTMP service listens to, and on which clients push and pull streams to lalserver.
    "rtmps_enable": true,                   // Whether to enable listening for the RTMPS service or not.
                                            // Note that RTMP and RTMPS can be independently turned on or off 
                                            // (i.e. either, both, or none can be turned on/off).
    "rtmps_addr": ":4935",                  // Port address on which the RTMPS service listens.
    "rtmps_cert_file": "./conf/cert.pem",   // Local cert file for RTMPS
    "rtmps_key_file": "./conf/key.pem",     // Local key file for RTMPS
    "gop_num": 0,                           // Number of GOP caches for RTMP pull streams.
                                            // This reduces the stream opening time, but may increase latency.
                                            // If 0, then no cache is used for sending.
    "single_gop_max_frame_num": 0,          // Maximum limit on the number of frames cached in a single GOP when GOP caching is enabled.
    "merge_write_size": 0                   // Merge small packets of data under this size (in bytes) into a single packet 
                                            // to improve server performance, but may cause lag.
                                            // If 0, do not merge.
  },
  "in_session": {                           // Configuration for all input streams (all protocols, all input types).
                                            // For details see: https://pengrl.com/lal/#/dummy_audio
    "add_dummy_audio_enable": false,        // Enable or disable dynamic detection of added muted AAC data.
                                            // If enabled, on any input stream that has exceeded `add_dummy_audio_wait_audio_ms` and still has not
                                            // received audio data, AAC data is automatically overlayed on this stream.
    "add_dummy_audio_wait_audio_ms": 150    // In milliseconds, see `add_dummy_audio_enable` for details
  },
  "default_http": {                         // Default configuration for HTTP listening.
                                            // If hls, httpflv, httpts do not have the following configuration items configured separately,
                                            // then the `default_http` configuration is used.
                                            // Note that whether the hls, httpflv, httpts services are enabled or not is not determined here.
    "http_listen_addr": ":8080",            // HTTP listener address.
    "https_listen_addr": ":4433",           // HTTPS listener address.
    "https_cert_file": "./conf/cert.pem",   // HTTPS local cert file path.
    "https_key_file": "./conf/key.pem"      // HTTPS local key file path.
  },
  "httpflv": {
    "enable": true,                         // Whether to enable listening for the HTTP-FLV service.
    "enable_https": true,                   // Whether to enable HTTPS-FLV listening.
    "url_pattern":"/",                      // The address of the pull stream URL routing path. 
                                            // Defaults to `/`, which means it is unrestricted and the route address can be any path.
                                            // If set to `/live/`, streams can only be pulled from the `/live/` path, e.g. `/live/test110.flv`.
    "gop_num": 0,                           // GOP caching for the httpflv protocol, the meaning is similar to `rtmp.gop_num`.
    "single_gop_max_frame_num": 0           // See `rtmp.single_gop_max_frame_num`.
  },
  "hls": {
    "enable": true,                         // Whether to enable listening for the HLS service.
    "enable_https": true,                   // Whether to enable HTTPS-HLS listening.
    "url_pattern": "/hls/",                 // Pull stream URL routing address, defaults to `/hls/`, the corresponding HLS (m3u8) pull stream URLs are:
                                            // - `/hls/{streamName}.m3u8`
                                            // - `/hls/{streamName}/playlist.m3u8`
                                            // - `/hls/{streamName}/record.m3u8`
                                            //
                                            // The `playlist.m3u8` file corresponds to live HLS, and only `<fragment_num>` TS file names 
                                            // are stored in the list, which is continuously incremented.
                                            // Newly generated TS files are added and outdated ones are removed.
                                            // The `record.m3u8` file corresponds to recording HLS, listing all TS files
                                            // from the first (oldest) TS file to the latest generated TS file; newly generated TS files
                                            // will be constantly appended to the list.
                                            // The address of the TS file uses the following template:
                                            // `/hls/{streamName}/{streamName}-{timestamp}-{index}.ts` or
                                            // `/hls/{streamName}-{timestamp}-{index}.ts`
                                            //
                                            // Note that the `url_pattern` of HLS cannot be the same as the `url_pattern` of httpflv or httpts.
    "out_path": "./lal_record/hls/",        // Output root directory for HLS m3u8 and files.
    "fragment_duration_ms": 3000,           // Duration of a single TS file fragment, in milliseconds.
    "fragment_num": 6,                      // Number of TS files in the `playlist.m3u8` file list.
    "delete_threshold": 6,                  // Deletion threshold for TS files.
                                            // Note that this is only used if `cleanup_mode` is 2,
                                            // meaning that only the most recent TS files removed from `playlist.m3u8` will be saved,
                                            // any TS files expired earlier will be deleted.
                                            // If not, the default value takes the value of `fragment_num`.
                                            // Note that this value should not be less than 1 to avoid deleting too fast and causing playback to fail.
    "cleanup_mode": 1,                      // HLS file cleanup mode:
                                            //
                                            // 0 — Do not delete m3u8+ts files, can be used for recording and other scenarios.
                                            //
                                            // 1 — Delete m3u8+ts files at the end of the input stream.
                                            // Note that the exact point in time for deletion is after the end of the push stream
                                            // `fragment_duration_ms * (fragment_num + delete_threshold)`
                                            // Deletion is delayed for a small period of time to avoid the pull side of HLS not finishing pulling
                                            // just after the input stream ends.
                                            //
                                            // 2 — Continuously delete out-of-date TS files during the push stream, keeping only the most recent ones
                                            // `delete_threshold + fragment_num + 1`.
                                            // And, at the end of the input stream, the cleanup mode 1 logic is also performed.
                                            //
                                            // Note that `record.m3u8` is only generated in modes 0 and 1.
                                            //
    "use_memory_as_disk_flag": false,       // Whether to use memory instead of disk to save m3u8+ts files.
                                            // Note that you should be careful about memory capacity when using this mode.
                                            // Generally it should not be used with `cleanup_mode` of 0 or 1.
    "sub_session_timeout_ms": 30000,        // The time in milliseconds after which the HLS player is determined to have timed out
                                            // and left when counting HLS player information.
                                            // If the value is set to 0, defaults to `fragment_num * fragment_duration_ms * 2`
    "sub_session_hash_key": "q191201771"    // Private key, used when calculating the unique ID of the player.
                                            // Note that if null, the counting of HLS player information is disabled.
  },
  "httpts": {
    "enable": true,                         // Whether to enable listening for the HTTP-TS service. Note that this is not TS in HLS,
                                            // but rather the persistent transmission of TS streams over a long HTTP connection.
    "enable_https": true,                   // Whether or not to enable HTTPS-TS listening.
    "url_pattern": "/",                     // The address of the pull stream URL routing path.
                                            // Defaults to `/`, which means it is unrestricted and can be any path address.
                                            // If set to `/live/`, streams can only be pulled from `/live/` paths, e.g. `/live/test110.ts`.
    "gop_num": 0,                           // See `rtmp.gop_num`.
    "single_gop_max_frame_num": 0           // See `rtmp.single_gop_max_frame_num`.
  },
  "rtsp": {
    "enable": true,                         // Whether to enable listening for the RTSP service.
    "addr": ":5544",                        // RTSP listening address.
    "rtsps_enable": true,                   // Whether or not to enable listening for the RTSPS service.
                                            // Note that RTSP and RTSPS can be independently turned on or off.
    "rtsps_addr": ":5322",                  // The port address on which the RTSPS service listens.
    "rtsps_cert_file": "./conf/cert.pem",   // RTSPS's local cert file.
    "rtsps_key_file": "./conf/key.pem",     // RTSPS's local key file.
    "out_wait_key_frame_flag": true,        // Whether or not to wait for the video keyframe data before sending it when RTSP sends data.
                                            //
                                            // This configuration item mainly determines the first frame, splash screen, audio/video synchronisation, etc.
                                            // If true, both audio and video will wait for the video keyframe to be sent.
                                            // (i.e., all audio or video is discarded and not sent until the video keyframe arrives)
                                            //
                                            // If false, send both audio and video directly (i.e., neither audio nor video waits 
                                            // for the video keyframe, neither waits for any data).
                                            //
                                            // Note that for audio-only streams, if this flag is true, the audio theoretically never 
                                            // waits for a video keyframe, i.e., the audio has no chance to be sent,
                                            // and if it is false, the audio is sent directly to the video.
                                            // To avoid this, lalserver will do its best to determine if the stream is audio-only.
                                            // If the stream is audio-only, the audio will be sent directly.
                                            // However, if there is a pure audio stream, it is still recommended to set this configuration item to false.
    "auth_enable": false,                   // Whether or not to enable authentication for the RTSP service.
    "auth_method": 1,                       // RTSP authentication method: 0 - Basic authentication method, 1 - Digest authentication method (MD5 algorithm)
    "username": "q191201771",               // RTSP service account, configuration is required to enable authentication.
    "password": "pengrl"                    // Password for RTSP service, required to enable authentication.
  },
  "record": {
    "enable_flv": true,                     // Whether to enable FLV recording.
    "flv_out_path": "./lal_record/flv/",    // FLV recording directory.
    "enable_mpegts": true,                  // Enable/disable MPEG-TS recording. Note that this is a long TS file recording, HLS recording is controlled by the HLS configuration above.
    "mpegts_out_path": "./lal_record/mpegts"    // MPEG-TS recording directory.
  },
  "relay_push": {
    "enable": false,                        // Whether to enable relay push; when enabled, all streams received by itself are forwarded out.
    "addr_list": [                          // The address of the relay to be forwarded, support multiple addresses, do 1 to n forwarding. Format example:
      "127.0.0.1:19351"
    ]
  },
  "static_relay_pull": {                    // Static return pull function (this service pulls a stream from elsewhere).
                                            // When lalserver receives a pull stream sub request and the input stream does not exist,
                                            // it will pull this stream locally from another server.
                                            // When there is no sub pull request, the pull request is automatically closed.
                                            // When a pull connection fails or is disconnected, it is automatically retried.
                                            // Note that if static pullbacks don't meet your needs, it's recommended to use the HTTP API for more customised pullbacks.
                                            // Note that you should not use both the configuration file-controlled static repo function
                                            // and the HTTP API-controlled repo function at the same time.
                                            // Also, the configuration items in `static_relay_pull` have nothing to do with the HTTP API.
    "enable": false,                        // Whether to enable this function or not. 
                                            // Note that only the static return function is controlled; if you are using the HTTP API,
                                            // then it needs to be turned off there instead.
    "addr": ""                              // The address of the pullback stream, in the format "127.0.0.1:19351".
                                            // Note that the URL path (i.e. the stream name, etc.) is obtained from the pull stream request itself.
  },
  "http_api": {
    "enable": true,                         // Whether to enable the HTTP API interface.
    "addr": ":8083"                         // Listening address (with port number).
  },
  "server_id": "1",                         // Unique ID of the current lalserver, which can be used to distinguish lalserver nodes
                                            // when multiple lalserver HTTP Notify callbacks are made to the same address.
  "http_notify": {                          // ----- HTTP-Notify documentation is available at: https://pengrl.com/lal/#/HTTPNotify
    "enable": true,                         // Whether to enable HTTP Notify event callbacks.
    "update_interval_sec": 5,               // Interval between update event callbacks, in seconds.
    "on_server_start": "http://127.0.0.1:10101/on_server_start",    // HTTP Notify event callback address for each event.
    "on_update": "http://127.0.0.1:10101/on_update",                // Note that this can be set to null for events that you don't care about.
    "on_pub_stop": "http://127.0.0.1:10101/on_pub_stop",
    "on_sub_start": "http://127.0.0.1:10101/on_sub_start",
    "on_sub_stop": "http://127.0.0.1:10101/on_sub_stop",
    "on_relay_pull_start": "http://127.0.0.1:10101/on_relay_pull_start",
    "on_relay_pull_stop": "http://127.0.0.1:10101/on_relay_pull_stop",
    "on_rtmp_connect": "http://127.0.0.1:10101/on_rtmp_connect"
  },
  "simple_auth": {                          // ----- See https://pengrl.com/lal/#/auth for authentication documentation.
    "key": "q191201771",                    // Private key, used to calculate MD5 authentication parameters.
    "dangerous_lal_secret": "pengrl",       // Backdoor authentication parameter, all streams can be authenticated with this value.
    "pub_rtmp_enable": false,               // Whether or not RTMP push streams are enabled for authentication.
                                            // True to enable authentication, false to disable authentication.
    "sub_rtmp_enable": false,               // If RTMP pull is enabled or not.
    "sub_httpflv_enable": false,            // Whether to enable authentication for httpflv pulls.
    "sub_httpts_enable": false,             // If HTTP-TS pull is enabled or not.
    "pub_rtsp_enable": false,               // If RTSP push is enabled or not.
    "sub_rtsp_enable": false,               // If RTSP pull is enabled or not.
    "hls_m3u8_enable": true                 // If m3u8 pull is enabled or not.
  },
  "pprof": {
    "enable": true,                         // Whether to enable listening to the Go pprof web service.
    "addr": ":8084"                         // Go pprof web address (with port number).
  },
  "log": {
    "level": 1,                             // Log level: 0 — trace, 1 — debug, 2 — info, 3 — warn, 4 — error, 5 — fatal.
    "filename": "./logs/lalserver.log",     // Log output file.
    "is_to_stdout": true,                   // Whether to print logs to the console.
    "is_rotate_daily": true,                // Logs are rolled over on a daily basis.
    "short_file_flag": true,                // Whether or not to display the source file name and line number at the end of the log.
    "assert_behavior": 1                    // The behaviour of the log assertion: 1 — prints only the error log,
                                            // 2 — prints and exits the application, 3 — prints and panics.
  },
  "debug": {
    "log_group_interval_sec": 30,           // Interval in seconds between printing group debug logs. If set to 0, logs are not printed.
    "log_group_max_group_num": 10,          // Maximum number of groups to be printed.
    "log_group_max_sub_num_per_group": 10   // The maximum number of sub sessions to print per group.
  }
}
```

## Descriptive version

### ▌ "# doc of config"

Type: string  
Value example: "https://pengrl.com/lal/#/ConfigBrief"

Link to the documentation for the config file. Not used by the application.

### ▌ "conf_version"

Type: string  
Example: "0.4.1"

Configuration file version number, should not be changed manually by the business side. The version is checked by the application to see if it is the same as the one declared in the code.

### ▌ "rtmp"

#### ✸ "rtmp/enable"

Type: bool  
Value example: true

Whether to enable listening to the RTMP service.  

Note that the enable switch in the configuration file that control each protocol type should be turned on only as needed to avoid unnecessary protocol conversion overhead.

#### ✸ "rtmp/addr"

Type: string  
Example value: "1935"

The port on which the RTMP service listens to, and on which clients push and pull streams to lalserver.

#### ✸ "rtmp/rtmps_enable"

Type: bool  
Value Example: true

Whether to enable listening for the RTMPS service or not.

Note that RTMP and RTMPS can be independently turned on or off (i.e. either, both, or none can be turned on/off).

#### ✸ "rtmp/rtmps_addr"

Type: string  
Example value: ":4935"

Port address on which the RTMPS service listens.

#### ✸ "rtmp/rtmps_cert_file"

Type: string  
Value example: `"./conf/cert.pem"`

Local cert file for RTMPS.

#### ✸ "rtmp/rtmps_key_file"

Type: string  
Value example: `"./conf/key.pem"`

Local key file for RTMPS.

#### ✸ "rtmp/gop_num"

Type: int  
Value example: 0

Number of GOP caches for RTMP pull streams. This reduces the stream opening time, but may increase latency.

If 0, then no cache is used for sending.

#### ✸ "rtmp/single_gop_max_frame_num"

Type: int  
Value example: 0

Maximum limit on the number of frames cached in a single GOP when GOP caching is enabled.

#### ✸ "rtmp/merge_write_size"

Type: int  
Value example: 0

Merge small packets of data under this size (in bytes) into a single packet to improve server performance, but may cause lag.

If 0, do not merge.

### ▌ "in_session"

Configuration for all input type streams (all protocols, all input types).

For details see: https://pengrl.com/lal/#/dummy_audio

#### ✸ "in_session/add_dummy_audio_enable"

Type: bool  
Value example: false

Enable or disable dynamic detection of added muted AAC data.

If enabled, on any input stream that has exceeded `add_dummy_audio_wait_audio_ms` and still has not received audio data, AAC data is automatically overlayed on this stream.

#### ✸ "in_session/add_dummy_audio_wait_audio_ms"

Type: int  
Value example: 150

In milliseconds, see `add_dummy_audio_enable` for details.

### ▌ "default_http"

Default configuration for HTTP listening.

If hls, httpflv, httpts do not have the following configuration items configured separately, then the `default_http` configuration is used.

Note that whether the hls, httpflv, httpts services are enabled or not is not determined here.

#### ✸ "default_http/http_listen_addr"

Type: string
Value Example: ":8080"

HTTP listener address.

#### ✸ "default_http/https_listen_addr"

Type: string
Value Example: ":4433"

HTTPS listener address.

#### ✸ "default_http/https_cert_file"

Type: string
Value Example: "./conf/cert.pem"

HTTPS local cert file path.

#### ✸ "default_http/https_key_file"

Type: string
Value Example: "./conf/key.pem"

HTTPS local key file address.


### ▌ "httpflv"

#### ✸ "httpflv/enable"

Type: bool  
Value Example: true

Whether to enable listening for the HTTP-FLV service.

#### ✸ "httpflv/enable_https"

Type: bool  
Value example: true

Whether to enable HTTPS-FLV listening.

#### ✸ "httpflv/url_pattern"

Type: string  
Value example: "/"

The address of the pull stream URL routing path.

Defaults to `/`, which means it is unrestricted and the route address can be any path.  
If set to `/live/`, streams can only be pulled from the `/live/` path, e.g. `/live/test110.flv`.

#### ✸ "httpflv/gop_num"

Type: int  
Value example: 0

GOP caching for the httpflv protocol, the meaning is similar to `rtmp.gop_num`.

#### ✸ "httpflv/single_gop_max_frame_num"

Type: int  
Value example: 0

See `rtmp.single_gop_max_frame_num`

### ▌ "hls"

#### ✸ "hls/enable"

Type: bool  
Value Example: true

Whether to enable listening for the HLS service.

#### ✸ "hls/enable_https"

Type: bool  
Value example: true

Whether to enable HTTPS-HLS listening.

#### ✸ "hls/url_pattern"

Type: string  
Value example: "/hls/"

Pull stream URL routing address, defaults to `/hls/`, the corresponding HLS (m3u8) pull stream URLs are:

- `/hls/{streamName}.m3u8`
- `/hls/{streamName}/playlist.m3u8`
- `/hls/{streamName}/record.m3u8`

The `playlist.m3u8` file corresponds to live HLS, only `<fragment_num>` TS file names are stored in the list, which is continuously incremented.  
Newly generated TS files are added and outdated ones are removed.

The `record.m3u8` file corresponds to recording HLS, listing all TS files from the first (oldest) TS file to the latest generated TS file; newly generated TS files will be constantly appended to the list.

The address of the TS file uses the following template:

- `/hls/{streamName}/{streamName}-{timestamp}-{index}.ts` or
- `/hls/{streamName}-{timestamp}-{index}.ts`.

Note that the `url_pattern` of HLS cannot be the same as the ` url_pattern` of httpflv or httpts.


#### ✸ "hls/out_path"

Type: string  
Value example: `"./lal_record/hls/"`

Output root directory for HLS m3u8 and files.


#### ✸ "hls/fragment_duration_ms"

Type: int  
Value example: 3000

Duration of a single TS file fragment, in milliseconds.


#### ✸ "hls/fragment_num"

Type: int  
Example values: 6

Number of TS files in the `playlist.m3u8` file list.

#### ✸ "hls/delete_threshold"

Type: int   
Value example: 6

Deletion threshold for TS files.

Note that it is only used if the value of the configuration item `cleanup_mode` is 2, meaning that only the most recent TS files removed from `playlist.m3u8` will be saved, any TS files expired earlier will be deleted.  
If not, the default value takes the value of the configuration item `fragment_num`.  
Note that the value should be not less than 1 to avoid deleting too fast and causing playback to fail.  

#### ✸ "hls/cleanup_mode"

Type: int  
Value example: 1

HLS file cleanup mode:

0 — Do not delete m3u8+ts files, can be used for recording and other scenarios.

1 — Delete m3u8+ts files at the end of the input stream.  
  Note that the exact point in time for deletion is after the end of the push stream  
  `fragment_duration_ms * (fragment_num + delete_threshold)`
  The reason for delaying the deletion for a small period of time is to avoid that the input stream has just finished and the pull side of the HLS has not finished pulling yet

2 Continuously delete outdated ts files during the push flow, keeping only the most recent
  `delete_threshold + fragment_num + 1`
  and keep only the latest `delete_threshold + fragment_num + 1` or so
  And, at the end of the input stream, the logic of cleanup mode 1 is also executed

Note that record.m3u8 is only generated in modes 0 and 1
``

#### ✸ "hls/use_memory_as_disk_flag"

Type: bool  
Value example: false

Whether to use memory instead of disk to save m3u8+ts files.  
Note that you should be careful about memory capacity when using this mode. Generally it should not be used with `cleanup_mode` of 0 or 1

xxx

#### ✸ "hls/sub_session_timeout_ms"

Type: int  
Value example: 30000

The time in milliseconds after which the HLS player is determined to have timed out and left when counting HLS player information.  
If value 0, the default value is `fragment_num` * `fragment_duration_ms` * 2

#### ✸ "hls/sub_session_hash_key"

Type: string  
Example value: "q191201771"

Private key, used when calculating the unique ID of the player.  
Note that if empty, the function of counting HLS player information is disabled

### ▌ "httpts"

#### ✸ "httpts/enable"

Type: bool  
Value Example: true

Whether or not to enable listening for the HTTP-TS service. Note that this is not TS in HLS, but rather the persistent transmission of TS streams over a long HTTP connection

#### ✸ "httpts/enable_https"

Type: bool  
Value Example: true

Whether to enable HTTPS-TS listening

#### ✸ "httpts/url_pattern"

Type: string  
Value example: "/"

Address of the pull stream URL routing path. The default value of `/` means that it is not restricted and the routing address can be any path address.  
If set to `/live/`, the stream can only be pulled from `/live/` path, such as `/live/test110.ts`.

#### ✸ "httpts/gop_num"

Type: int  
Value example: 0

See rtmp.gop_num

#### ✸ "httpts/single_gop_max_frame_num"

Type: int  
Value example: 0

See rtmp.single_gop_max_frame_num

### ▌ "rtsp"

TODO

### ▌ "record"

TODO

### ▌ "relay_push"

TODO

### ▌ "static_relay_pull"

TODO

### ▌ "http_api"

TODO

### ▌ "server_id"

TODO

### ▌ "http_notify"

TODO

### ▌ "simple_auth"

TODO

### ▌ "pprof"

TODO

### ▌ "log"

TODO

### ▌ "debug"

TODO
