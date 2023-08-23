# lalserver HTTP API interfaces

lalserver provides a HTTP API which you can use to get some status of lalserver and control some of its behaviours.

## ▌ I. Related Configuration

The HTTP API has some entries in the configuration file as follows (specifically based on the document [lalserver configuration file description](ConfigBrief.md):

```json
  ...
  "http_api": {
    "enable": true,    // Whether to enable the HTTP API interface.
    "addr": ":8083"    // Listening address.
  },
  ...
```

## ▌ II. List of Interfaces

Interfaces are divided into two main categories:

1. Query types, starting with `/api/stat`.
2. Control types, starting with `/api/ctrl`.

- 1.1. `/api/stat/group`     — Query information about a specific group.
- 1.2. `/api/stat/all_group` — Query information about all groups.
- 1.3. `/api/stat/lal_info`  — Query server information.

- 2.1. `/api/ctrl/start_relay_pull` — Control the server to pull streams from a remote location to the local one
- 2.2. `/api/ctrl/stop_relay_pull`  — Stop the relay pull.
- 2.3. `/api/ctrl/kick_session`     — Forcefully kick out and close the specified session, which can be of type pub, sub, or pull.
- 2.4. `/api/ctrl/start_rtp_pub`    — Open the GB28181 receive port.
- 2.5. `/api/ctrl/stop_rtp_pub`     — Close the GB28181 receive port.


## ▌ III. Nomenclature Explanation:

- **group:** lal, as a streaming server, sends every 1 input stream to `1~n` output streams by stream name; input and output streams with the same stream name are considered to be managed by the same group.

## ▌ IV. Interface Rules

1. Mandatory first-level parameters to be included in the return result of all interfaces:

```json
{
    "error_code": 0,
    "desp": "succ",
    "data": "..."
}
```

2. The `error_code` list:

| `error_code` | desp                        | description                |
|-------------:|-----------------------------|----------------------------|
|            0 | succ                        | Call successful            |
|         1001 | group not found             | Group does not exist       |
|         1002 | param missing               | Required parameter missing |
|         1003 | session not found           | Session does not exist     |
|         2001 | multiple values for specific reasons for failure | `start_relay_pull` failure |
|         2002 | failed to open port GB28181 | `start_rtp_pub` failure    |

3. Note that some interfaces are called using HTTP GET + URL parameters, and some interfaces are called using HTTP POST + JSON body; please check the documentation carefully.

## ▌ V. Interface Details

### 1.1 `/api/stat/groups`

✸ Brief description: Query the information for a specific group.

✸ Example request:

```shell
$ curl http://127.0.0.1:8083/api/stat/group?stream_name=test110
```
✸ Request method: `HTTP GET` + URL parameter

✸ Request parameters:

- stream_name | mandatory | stream name for the specified group

✸ Return value `error_code` possible values:

- **0** Group exists, query succeeded
- **1001** Group does not exist
- **1002** Missing mandatory parameter

✸ Return example:

```json
                                
{
  "error_code": 0,                  // Interface return value, 0 means success.
  "desp": "succ",                   // Interface return description, "succ" means success.
  "data": {
    "stream_name": "test110",       // stream name
    "app_name": "live",             // appName
    "audio_codec": "AAC",           // audio codec format "AAC"
    "video_codec": "H264",          // video codec format "H264" | "H265"
    "video_width": 640,             // video width
    "video_height": 360,            // video height
    "pub": { // ----- receives information about the push stream -----
      "session_id": "RTMPPUBSUB1",  // Session ID, a globally unique identifier for the session.
      "protocol": "RTMP",           // Push stream protocol, range: "RTMP" | "RTSP"
      "base_type": "PUB",           // Base type, fixed to "PUB".
      "start_time": "2020-10-11 19:17:41.586", // Start time of push flow
      "remote_addr": "127.0.0.1:61353",        // peer address
      "read_bytes_sum": 9219247,    // Cumulative read data size (counted from start of push)
      "wrote_bytes_sum": 3500,      // Cumulative sent data size
      "bitrate_kbits": 436,         // Last 5 seconds bitrate in kbit/s. Equivalent to `read_bitrate_kbits` for pub types unless otherwise declared.
      "read_bitrate_kbits": 436,    // Last 5 seconds of read data bitrate
      "write_bitrate_kbits": 0      // Last 5 seconds of transmit data rate.
    },
    "subs": [ // Information about ----- pulling streams, there may be multiple protocols, each with multiple possible session connections -----
      {
        "session_id": "FLVSUB1",    // session ID, globally unique identifier for the session
        "protocol": "FLV",          // pull streaming protocol, range: "RTMP" | "FLV" | "TS"
        "base_type": "SUB",         // Base type, fixed to "SUB".
        "start_time": "2020-10-11 19:19:21.724", // Start time of stream pulling
        "remote_addr": "127.0.0.1:61785", // peer address
        "read_bytes_sum": 134,      // Cumulative read data size (calculated from the start of the pull)
        "wrote_bytes_sum": 2944020, // Cumulative size of sent data
        "bitrate_kbits": 439,       // Last 5 seconds bitrate in kbit/s. For sub types, this is equivalent to `write_bitrate_kbits` unless otherwise declared.
        "read_bitrate_kbits": 0,    // Last 5 seconds of read data bitrate
        "write_bitrate_kbits": 439  // the last 5 seconds of send data bitrate
      }
    ],
    "pull": { // ----- This node pulls streams from other nodes back to the source -----
      "base_type": "PULL",      // This is fixed to "PULL".
      ...                       // The other fields are the same as the pub's internal fields above, so I won't go into them again.
    },
    "pushs": []                 // Active outbound push information, not provided yet.
  }
}
```

### 1.2 `/api/stat/all_groups`

✸ Brief description: Query information about all groups.

✸ Example request:

```shell
$ curl http://127.0.0.1:8083/api/stat/all_group
```
✸ Request method: `HTTP GET`.

✸ Request parameters: none

✸ Return value `error_code` Possible values:

- **0** The query succeeded

✸ Return example:

```json
{
    "error_code": 0,
    "desp": "succ",
    "data": {
        "groups": [
            ...      // The content of each element in the array is formatted in the same way as the data field in the /api/stat/group interface, so we won't go into that again.
        ]
    }
}
```

### 1.3 `/api/stat/lal_info`

✸ Brief description: Query server information.

✸ Example request:

```shell
$curl http://127.0.0.1:8083/api/stat/lal_info
```

✸ Request method: `HTTP GET`

✸ Request parameters: none

✸ Return value `error_code` possible values:

- **0** The query succeeded.

✸ Return example:

```json
{
  "error_code": 0,
  "desp": "succ",
  "data": {
    "server_id": "1",
    "bin_info": "GitTag=v0.17.0. GitCommitLog=bbf850aca2d4f3e55380d44ca9c3a16be60c8d39 ${NewVersion} -> version.go. GitStatus= M CHANGELOG.md | M gen_tag.sh | M pkg/base/version.go. BuildTime=2020.11.21.173812. GoVersion=go version go1.14.2 darwin/amd64. runtime=darwin/amd64.",
    "lal_version": "v0.17.0",   // lal executable version information
    "api_version": "v0.1.2",    // HTTP API version information.
    "notify_version": "v0.0.4", // HTTP Notify version information
    "start_time": "2020-11-21 17:34:53.973" // lal process start time
  }
}
```

### 2.1 `/api/ctrl/start_relay_pull`

✸ Brief description: Controls the server's active pulling of streams from the remote end to the local end.

✸ Example request:

```shell
$ curl -H "Content-Type:application/json" -X POST -d '{"url": "rtmp://127.0.0.1/live/test110?token=aaa&p2=bbb", "pull_retry_num": 0}' http://127.0.0.1:8083/api/ctrl/start_relay_pull
```

✸ request method: `HTTP POST`

✸ Request parameters:

```json
{
    "url": "rtmp://127.0.0.1/live/test110?token=aaa&p2=bbb", // Mandatory, the full URL address of the source pull stream, RTMP and RTSP are currently supported.
    "stream_name": "test110",   // Optional; if not specified, parse from the `url` parameter.
    "pull_timeout_ms": 10000,   // Optional: the timeout in milliseconds for pull to establish a session.
                                // Default value is 10000.
    "pull_retry_num": 0,        // Optionally, the number of times a pull connection fails before it is disconnected.
                                // -1 means keep retrying until you get a stop
                                // request, or enable and trigger the auto-shutdown feature below.
                                // = 0 means no retries
                                // > 0 means retries
                                // The default value is 0.
                                // Tip: Without auto-reconnect turned on, you can
                                // decide whether to reconnect or not when you
                                // receive HTTP-Notify `on_relay_pull_stop`, 
                                // `on_update`, etc. messages.
    "auto_stop_pull_after_no_out_ms": -1, // Option to `auto_stop` the pull session after the last viewer exited in order to save resources.
                                // -1 means don't activate this feature.
                                // = 0 means close the pull session immediately when there are no viewers.
                                // > 0 means close the pull session when there are no // viewers, in milliseconds.
                                // The default value is -1.
                                // Tip: Leave this feature on so you can decide 
                                // whether to close the relay pull when you receive
                                // HTTP-Notify `on_sub_stop`, `on_update`, etc. messages.
    "rtsp_mode": 0,             // Optionally, the connection mode when using RTSP:
                                // 0 tcp
                                // 1 udp
                                // Default value is 0.
    "debug_dump_packet": ""     // Option to dump the received data to a file.
                                // Be careful, use it only when there is a problem, // and provide the stored file to the lal author for 
                                // analysis. Turn it off when there are no problems // to avoid performance degradation and wasted disk space.
                                // Example values: "./dump/test110.laldump", "/tmp/test110.laldump"
                                // If set to the empty string "", no file will be saved.
                                // The default value is "".
}
```

✸ The return value `error_code` may take the following values:

- **0** The request was successful.
- **1002** Parameter error.
- **2001** The request failed, refer to `desp` for the failure description.
  - "lal.logic: in stream already exists in group": input stream already exists.

> Note: Returning success indicates that lalserver received the command and started pulling the stream from the remote end, it does not guarantee the success of pulling the stream from the remote end. You can use HTTP-Notify callback events such as `on_relay_pull_start`, `on_update`, etc. to determine whether the stream pull is successful or not.

✸ Return example:

```json
{
  "error_code": 0,
  "desp": "succ",
  "data": {
    "stream_name": "test110",
    "session_id": "RTMPPULL1"
  }
}
```

### 2.2 `/api/ctrl/stop_relay_pull`

✸ Brief description: Stop a specific relay pull.

✸ Example request:

```shell
$ curl http://127.0.0.1:8083/api/ctrl/stop_relay_pull?stream_name=test110
```

✸ Request method: `HTTP GET` + URL parameter

✸ Request parameters:

- stream_name | mandatory | name of the stream for which the relay pull needs to be turned off

✸ Return value `error_code` possible values:

- **0** group exists, query succeeded
- **1001** group does not exist
- **1002** mandatory parameter is missing
- **1003** pull session does not exist

✸ Return example:

```json
{
  "error_code": 0
  "desp": "succ",
  "data": {
    "session_id": "RTMPPULL1"
  }
}
```

> Hint: In addition to `stop_relay_pull`, you can also use `kick_session` to turn off the relay pull back to the source pull stream.

### 2.3 `/api/ctrl/kick_session`

✸ Brief description: Force kick to close the specified session. Session can be of type `pub`, `sub`, `pull`.

✸ Example request:

```shell
$ curl -H "Content-Type:application/json" -X POST -d '{"stream_name": "test110", "session_id": "FLVSUB1"}' http://127.0.0.1:8083/api/ctrl/kick_session
```

✸ Request method: `HTTP POST`

✸ Request parameters:

```json
{
  "stream_name": "test110", // mandatory, stream name
  "session_id": "FLVSUB1"   // mandatory, unique session identifier
}
```

✸ The return value `error_code` may return the following:

- **0** The request interface was successful. The specified session was closed.
- **1001** The group corresponding to the specified stream name does not exist.
- **1002** Parameter error.
- **1003** The specified session does not exist.

✸ Return Example:

```json
{
  "error_code": 0,
  "desp": "succ"
}
```

### 2.4 `/api/ctrl/start_rtp_pub`

✸ Brief description: Open the GB28181 receive port.

✸ Example request:

```shell
$ curl -H "Content-Type:application/json" -X POST -d '{"stream_name": "test110", "port": 0, "timeout_ms": 10000}' http://127.0.0.1:8083/api/ctrl/start_rtp_pub
```

✸ Request method: `HTTP POST`

✸ Request parameters:

```json
{
  "stream_name": "test110", // Mandatory; the name of the stream to which subsequent streams are bound, e.g. the name of the generated recording file, the name of the stream that pulls streams using other protocols, etc.
  "port": 0,                // Optional: receive port.
                            // If 0, lalserver chooses a random port and returns the 
                            // port to the caller with the return value
                            // The default value is 0.
  "timeout_ms": 60000,      // Optional, timeout in milliseconds, if no data is received on or after this timeout, the port will be closed.
                            // If 0, the port will not be closed after this timeout.
                            // Defaults to 60000.
  "is_tcp_flag": 0,         // Optionally, whether to use TCP for streaming audio/video data.
                            // If 1, use TCP; if 0, use UDP.
                            // Default value is 0.
  "debug_dump_packet": "",  // Option to dump received UDP data to a file.
                            // Be careful, use it only when there is a problem, and // provide the stored file to the lal author for analysis.// Turn it off when there are no problems to avoid 
                            // performance degradation and wasted discs.
                            // Example values: "./dump/test110.laldump", 
                            // "/tmp/test110.laldump".
                            // If it is the empty string "", no file will be saved.
                            // The default value is "".
}
```

✸ The return value `error_code` may return the following:

- **0** The request was successful. Port opened successfully.
- **1002** Parameter error.
- **2002** Binding listening port failed.

✸ Return example:

```json
{
  "error_code": 0
  "desp": "succ",
  "data": {
    "stream_name": "test110",
    "session_id": "pssub1",
    "port": 20000
  }
}
```

### 2.5 `/api/ctrl/stop_rtp_pub`

At this stage, please use the `/api/ctrl/kick_session` request to perform the proactive shutdown of RTP pubs.

The `stop_rtp_pub` interface is not available at this time.

NOTE: HttpApiVersion = "v0.4.6"
