# lalserver HTTP Notify(Callback/Webhook)事件回调

lalserver作为流媒体服务器，可将内部的一些事件通过HTTP POST的方式，传递给业务方。  
业务方结合lalserver的HTTP Notify和 [HTTP API接口](HTTPAPI.md)，可以更好的实现自己的定制化业务。  

## ▌ 一. 相关配置

HTTP Notify在配置文件中有一些配置如下（具体以 [lalserver 配置文件说明](https://pengrl.com/lal/#/ConfigBrief)  这个文档为准）：

```
  "server_id": "1", //. 当前lalserver唯一ID。多个lalserver HTTP Notify向同一个地址回调时，可通过该ID区分lalserver节点
  "http_notify": {
    "enable": true,                                              //. 是否开启HTTP Notify事件回调
    "update_interval_sec": 5,                                    //. update事件回调间隔，单位秒
    "on_server_start": "http://127.0.0.1:10101/on_server_start", //. 各事件HTTP Notify事件回调地址
    "on_update": "http://127.0.0.1:10101/on_update",             //  注意，对于不关心的事件，可以设置为空
    "on_pub_start": "http://127.0.0.1:10101/on_pub_start",
    "on_pub_stop": "http://127.0.0.1:10101/on_pub_stop",
    "on_sub_start": "http://127.0.0.1:10101/on_sub_start",
    "on_sub_stop": "http://127.0.0.1:10101/on_sub_stop",
    "on_relay_pull_start": "http://127.0.0.1:10101/on_relay_pull_start",
    "on_relay_pull_stop": "http://127.0.0.1:10101/on_relay_pull_stop",
    "on_rtmp_connect": "http://127.0.0.1:10101/on_rtmp_connect"
  },
```

## ▌ 二. HTTP Notify列表

```
 1. on_update           // 定时汇报所有group、session的信息
 2. on_pub_start        // 别人推流到当前节点
 3. on_pub_stop         // 推流停止
 4. on_sub_start        // 别人从当前节点拉流
 5. on_sub_stop         // 拉流停止
 6. on_relay_pull_start // 回源拉流成功
 7. on_relay_pull_stop  // 回源拉流停止
 8. on_rtmp_connect     // 收到rtmp connect message信令
 9. on_server_start     // 服务启动时
10. on_hls_make_ts      // hls生成每个ts分片文件时
```

## ▌ 三. 示例

所有接口使用HTTP POST方式，下面是`on_pub_start`的一个示例：

```
POST /on_pub_start HTTP/1.1
Host: 127.0.0.1:10101
User-Agent: Go-http-client/1.1
Content-Length: 261
Content-Type: application/json
Accept-Encoding: gzip

{"server_id":"1","protocol":"RTMP","url":"rtmp://127.0.0.1:1935/live/test110?token=1111","app_name":"live","stream_name":"test110","url_param":"token=1111","session_id":"RTMPPUBSUB1","remote_addr":"127.0.0.1:52576","has_in_session":true,"has_out_session":false}
```

## ▌ 四. 接口详情

### 1 `on_update`

定时汇报所有group、session的信息，间隔时间由配置文件的配置决定

```
{
    "server_id": "1", // 配置文件中配置。当有多个lalserver向同一个业务系统地址notify时，通过该字段区分不同lalserver节点
    groups: [
      {
        "stream_name": "test110", // 流名称
        "app_name":    "live",    // appName
        "audio_codec": "AAC",     // 音频编码格式 "AAC"
        "video_codec": "H264",    // 视频编码格式 "H264" | "H265"
        "video_width": 640,       // 视频宽
        "video_height": 360,      // 视频高
        "pub": {                                             // -----接收推流的信息-----
          "session_id": "RTMPPUBSUB1",                       // 会话ID，会话全局唯一标识
          "protocol": "RTMP",                                // 推流协议，取值范围： "RTMP" | "RTSP"
          "base_type": "PUB",                                // 基础类型，该处固定为"PUB"
          "start_time": "2024-05-16 14:50:34.795 +0800 CST", // 推流开始时间
          "remote_addr": "127.0.0.1:61353",                  // 对端地址
          "read_bytes_sum": 9219247,                         // 累计读取数据大小（从推流开始时计算）
          "wrote_bytes_sum": 3500,                           // 累计发送数据大小
          "bitrate_kbits": 436,                              // 最近5秒码率，单位kbit/s。对于pub类型，如无特殊声明，等价于`read_bitrate_kbits`
          "read_bitrate_kbits": 436,                         // 最近5秒读取数据码率
          "write_bitrate_kbits": 0                           // 最近5秒发送数据码率
        },
        "subs": [                                              // -----拉流的信息，可能存在多种协议，每种协议可能存在多个会话连接-----
          {
            "session_id": "FLVSUB1",                           // 会话ID，会话全局唯一标识
            "protocol": "FLV",                                 // 拉流协议，取值范围： "RTMP" | "FLV" | "TS"
            "base_type" "SUB"                                  // 基础类型，该处固定为"SUB"
            "start_time": "2024-05-16 14:50:34.795 +0800 CST", // 拉流开始时间
            "remote_addr": "127.0.0.1:61785",                  // 对端地址
            "read_bytes_sum": 134,                             // 累计读取数据大小（从拉流开始时计算）
            "wrote_bytes_sum": 2944020,                        // 累计发送数据大小
            "bitrate_kbits": 439,                              // 最近5秒码率，单位kbit/s。对于sub类型，如无特殊声明，等价于`write_bitrate_kbits`
            "read_bitrate_kbits": 0,                           // 最近5秒读取数据码率
            "write_bitrate_kbits": 439                         // 最近5秒发送数据码率
          }
        ],
        "pull": {              // -----该节点从其他节点拉流回源信息-----
          "base_type": "PULL", // 该处固定为"PULL"
          ...                  // 其他字段和上面pub的内部字段相同，不再赘述
        }, 
        "pushs":[], // 主动外连转推信息，暂时不提供
        "in_frame_per_sec": [  // 最近32秒内的fps（也即每一秒有多少帧视频数据）
          {
            "unix_sec": 1723513826,
            "v": 15
          },
          {
            "unix_sec": 1723513825,
            "v": 15
          },
          {
            "unix_sec": 1723513824,
            "v": 15
          }
        ]
      }
    ]
}
```

### 2 `on_pub_start`

别人推流到当前节点。目前支持RTMP、RTSP，示例如下：

```
1) rtmp
$ffmpeg -re -i wontcry.flv -c:a copy -c:v copy -f flv "rtmp://127.0.0.1:1935/live/test110?token=1111"

{
    "server_id": "1",                                       // 该lalserver节点的id，与配置文件中的server_id字段相同
    "session_id": "RTMPPUBSUB5",                            // 会话ID，会话全局唯一标识
    "protocol": "RTMP",                                     // 推流协议，取值范围： "RTMP" | "RTSP"
    "base_type": "PUB",                                     // 基础类型，该处固定为"PUB"
    "remotet_addr": "127.0.0.1:52010"                       // 连接对端地址
    "url": "rtmp://127.0.0.1:1935/live/test110?token=1111", // 完整url
    "app_name": "live",                                     // RTMP URL中的app name
    "stream_name": "test110",                               // 流名称
    "url_param": "token=1111",                              // URL参数
    "has_in_session": true,                                 // 对应的group中，是否存在输入音视频数据的流
    "has_out_session": false,                               // 对应的group中，是否存在输出音视频数据的流
    "read_bytes_sum": 3407,                                 // 会话建立至当前时间点，总共读取的数据，单位字节
    "wrote_bytes_sum": 3500                                 // 会话建立至当前时间点，总共发送的数据，单位字节
}

-----

2) rtsp
$ffmpeg -re -i wontcry.flv -c:a copy -c:v copy -f rtsp rtsp://localhost:5544/live/test110

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

推流停止。字段同`on_pub_start`，不再赘述。

### 4 `on_sub_start`

别人从当前节点拉流。目前支持RTMP、RTSP、`HTTP-FLV`、`HTTP-TS`、HLS，示例如下：

```
1) HTTP-FLV
$wget http://127.0.0.1:8080/live/test110.flv\?token\=1111

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

2) HTTP-TS
$wget http://127.0.0.1:8080/live/test110.ts\?token\=1111

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

RTMP、RTSP等其他协议类似，不再赘述。
```

关于按需回源拉流：

- 业务方可以在收到`on_sub_start`事件，并且`has_in_session`为false时（说人话：有人来看，但是流不存在），通过HTTP API接口start_relay_pull触发lalserver从第三方拉流
- 业务方可以在收到`on_sub_stop`事件，并且`has_out_session`为false时（说人话：人走了，没人看了），通过HTTP API接口触发lalserver停止从第三方拉流

### 5 `on_sub_stop`

拉流停止。字段同`on_sub_start`，不再赘述。

### 6 `on_relay_pull_start`

回源拉流成功。

```
{
    "server_id": "1",                                       //
    "session_id": "RTMPPULL1",                              // 会话ID，会话全局唯一标识
    "protocol": "RTMP",                                     // 协议，取值范围： "RTMP" | "RTSP"
    "base_type": "PULL",                                    // 基础类型，该处固定为"PULL"
    "remotet_addr": "127.0.0.1:1935"                        // 连接对端地址
    "url": "rtmp://127.0.0.1:1935/live/test110?token=1111", // 完整url
    "app_name": "live",                                     // RTMP URL中的app name
    "stream_name": "test110",                               // 流名称
    "url_param": "token=1111",                              // URL参数
    "has_in_session": true,                                 // 对应的group中，是否存在输入音视频数据的流
    "has_out_session": false                                // 对应的group中，是否存在输出音视频数据的流
    "read_bytes_sum": 0,                                    // 会话建立至当前时间点，总共读取的数据，单位字节
    "wrote_bytes_sum": 0                                    // 会话建立至当前时间点，总共发送的数据，单位字节
}
```

### 7 `on_relay_pull_stop`

回源拉流停止。字段同`on_relay_pull_stop`，不再赘述。

### 8 `on_rtmp_connect`

收到rtmp connect message信令。

```
{
  "server_id":"1",
  "session_id":"RTMPPUBSUB1",
  "remote_addr":"127.0.0.1:53834",
  "app":"live",
  "flashVer":"FMLE/3.0 (compatible; Lavf57.83.100)",
  "tcUrl":"rtmp://127.0.0.1:19350/live"
}
```

### 9 `on_server_start`

服务启动时。字段同HTTP API中`/api/stat/lal_info`中的data字段。

### 10 `on_hls_make_ts`

hls生成每个ts分片文件时。

```
{
  "event": "open",                                                // . "open"  表示ts分片文件被创建
                                                                  //   "close" 表示ts分片文件写入完毕
                                                                  //
  "stream_name": "test110"                                        // . 流名称
  "cwd": "/Volumes/T7/lal",                                       // . 当前工作路径。
                                                                  //   当配置文件中的hls目录设置为相对路径时，相对的是该字段的值
  "ts_file": "lal_record/hls/test110/test110-1654601694357-2.ts", // . ts文件磁盘地址
  "live_m3u8_file": "lal_record/hls/test110/playlist.m3u8",       // . 直播m3u8文件磁盘地址
  "record_m3u8_file": "lal_record/hls/test110/record.m3u8",       // . 录制m3u8文件磁盘地址
                                                                  //   注意，只有开启录制时，该字段才有意义
                                                                  //
  "id": 2,                                                        // . ts文件的ID编号。线性递增
  "duration": 3.333,                                              // . ts文件的时长，单位秒
                                                                  //   注意，只有event为"close"时该字段才有意义
                                                                  //
  "server_id": "1"                                                // .
}
```

NOTE: HttpNotifyVersion = "v0.2.5"
