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
            "audio_codec": "AAC",     // 音频编码格式 "AAC"
            "video_codec": "H264",    // 视频编码格式 "H264" | "H265"
            "video_width": 640,       // 视频宽
            "video_height": 360,      // 视频高
            "pub": {                  // 接收推流的信息
              "protocol": "RTMP",                      // 推流协议 "RTMP" | "RTSP"
              "session_id": "RTMPPUBSUB1",             // 会话ID，会话全局唯一标识
              "start_time": "2020-10-11 19:17:41.586", // 推流开始时间
              "remote_addr": "127.0.0.1:61353",        // 对端地址
              "read_bytes_sum": 9219247,               // 累计读取数据大小（从推流开始时计算）
              "wrote_bytes_sum": 3500,                 // 累计发送数据大小
              "bitrate": 436,                          // 最近5秒码率，单位kbit/s。对于pub类型，如无特殊声明，等价于`read_bitrate`
              "read_bitrate": 436,                     // 最近5秒读取数据码率
              "write_bitrate": 0                       // 最近5秒发送数据码率
            },
            "subs": [                 // 拉流的信息，可能存在多种协议，每种协议可能存在多个会话连接
              {
                "protocol": "HTTP-FLV",                  // 拉流协议 "RTMP" | "HTTP-FLV" | "HTTP-TS"
                "session_id": "FLVSUB1",                 // 会话ID，会话全局唯一标识
                "start_time": "2020-10-11 19:19:21.724", // 拉流开始时间
                "remote_addr": "127.0.0.1:61785",        // 对端地址
                "read_bytes_sum": 134,                   // 累计读取数据大小（从拉流开始时计算）
                "wrote_bytes_sum": 2944020,              // 累计发送数据大小
                "bitrate": 439,                          // 最近5秒码率，单位kbit/s。对于sub类型，如无特殊声明，等价于`write_bitrate`
                "read_bitrate": 0,                       // 最近5秒读取数据码率
                "write_bitrate": 439                     // 最近5秒发送数据码率
              }
            ],
            "pull": {}, // 该节点从其他节点拉流回源信息，内部字段和上面pub的内部字段相同，不再赘述
            "pushs":[]  // 主动外连转推信息，暂时不提供
        }
    ]
}
```

### 2 `on_pub_start`

目前支持RTMP、RTSP，示例如下：

```
1) rtmp
$ffmpeg -re -i wontcry.flv -c:a copy -c:v copy -f flv "rtmp://127.0.0.1:1935/live/test110?token=1111"

{
    "server_id": "1",                                       // 该lalserver节点的id，与配置文件中的server_id字段相同
    "protocol": "RTMP",                                     // 协议类型
    "url": "rtmp://127.0.0.1:1935/live/test110?token=1111", // 完整url
    "app_name": "live",                                     // RTMP URL中的app name
    "stream_name": "test110",                               // 流名称
    "url_param": "token=1111",                              // URL参数
    "session_id": "RTMPPUBSUB5",                            // 连接唯一ID
    "remotet_addr": "127.0.0.1:52010"                       // 连接对端地址
    "has_in_session": true,                                 // 对应的group中，是否存在输入音视频数据的流
    "has_out_session": false                                // 对应的group中，是否存在输出音视频数据的流
}

-----

2) rtsp
$ffmpeg -re -i wontcry.flv -c:a copy -c:v copy -f rtsp rtsp://localhost:5544/live/test110

{
    "server_id": "1",
    "protocol": "RTSP",
    "url": "rtsp://localhost:5544/live/test110",
    "app_name": "live",
    "stream_name": "test110",
    "url_param": "",
    "session_id": "RTSPPUB2",
    "remotet_addr": "[::1]:57308"
    "has_in_session": true,
    "has_out_session": false
}
```

### 3 `on_pub_stop`

字段同`on_pub_start`，不再赘述。

### 4 `on_sub_start`

目前支持RTMP、RTSP、`HTTP-FLV`、`HTTP-TS`，示例如下：

```
1) HTTP-FLV
$wget http://127.0.0.1:8080/live/test110.flv\?token\=1111

{
    "server_id": "1",
    "protocol": "HTTP-FLV",
    "url": "http://127.0.0.1:8080/live/test110.flv?token=1111",
    "app_name": "live",
    "stream_name": "test110",
    "url_param": "token=1111",
    "session_id": "FLVSUB1",
    "remotet_addr": "127.0.0.1:51889",
    "has_in_session": false,
    "has_out_session": true
}

2) HTTP-TS
$wget http://127.0.0.1:8082/live/test110.ts\?token\=1111

{
    "server_id": "1",
    "protocol": "HTTP-TS",
    "url": "http://127.0.0.1:8082/live/test110.ts?token=1111",
    "app_name": "live",
    "stream_name": "test110",
    "url_param": "token=1111",
    "session_id": "TSSUB1",
    "remotet_addr": "127.0.0.1:52170"
    "has_in_session": false,
    "has_out_session": true
}

RTMP、RTSP类似，不再赘述。
```

关于按需回源拉流：

- 业务方可以在收到`on_sub_start`事件，并且`has_in_session`为false时（说人话：有人来看，但是流不存在），通过HTTP API接口start_relay_pull触发lalserver从第三方拉流
- 业务方可以在收到`on_sub_stop`事件，并且`has_out_session`为false时（说人话：人走了，没人看了），通过HTTP API接口触发lalserver停止从第三方拉流

### 5 `on_sub_stop`

字段同`on_sub_start`，不再赘述。

### 6 `on_relay_pull_start`

```
{
    "server_id": "1",                                       //
    "session_id": "RTMPPULL1",                              // 连接唯一ID
    "url": "rtmp://127.0.0.1:1935/live/test110?token=1111", // 完整url
    "protocol": "RTMP",                                     // 协议类型
    "app_name": "live",                                     // RTMP URL中的app name
    "stream_name": "test110",                               // 流名称
    "url_param": "token=1111",                              // URL参数
    "remotet_addr": "127.0.0.1:1935"                        // 连接对端地址
    "has_in_session": true,                                 // 对应的group中，是否存在输入音视频数据的流
    "has_out_session": false                                // 对应的group中，是否存在输出音视频数据的流
}
```

### 7 `on_relay_pull_stop`

字段同`on_relay_pull_stop`，不再赘述。

### 8 `on_rtmp_connect`

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

字段同HTTP API中`/api/stat/lal_info`中的data字段。
