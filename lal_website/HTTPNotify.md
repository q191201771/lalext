# lalserver HTTP Notify(Callback/Webhook)事件回调

lalserver作为流媒体服务器，可将内部的一些事件通过HTTP POST的方式，传递给业务方。  
业务方结合lalserver的HTTP Notify和[《HTTP API接口》](HTTPAPI.md)，可以更好的实现自己的定制化业务。  
业务方的URL地址在配置文件中`http_notify`块中配置，具体见：[《配置文件说明》](CommonBrief.md)

## 一. HTTP Notify列表

<!-- more -->

```
1. on_update       // 定时汇报所有group、session的信息
2. on_pub_start    // 别人推流到当前节点
3. on_pub_stop     // 推流停止
4. on_sub_start    // 别人从当前节点拉流
5. on_sub_stop     // 拉流停止
6. on_rtmp_connect // 收到rtmp connect message信令
7. on_server_start // 服务启动时
```

## 二. 示例

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

## 三. 接口详情

### 1 `on_update`

定时汇报所有group、session的信息

```
{
    "server_id": "1", // 配置文件中配置。当有多个lalserver向同一个业务系统地址notify时，通过该字段区分不同lalserver节点
    groups: []        // 见HTTP API中/api/stat/all_group接口的data/groups字段
}
```

### 2 `on_pub_start`

目前支持RTMP、RTSP，示例如下：

```
1) rtmp
$ffmpeg -re -i wontcry.flv -c:a copy -c:v copy -f flv "rtmp://127.0.0.1:1935/live/test110?token=1111"

{
    "server_id": "1",                                       //
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
    "remotet_addr": "127.0.0.1:51889"
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

- 业务方可以在收到`on_sub_start`事件，并且`has_in_session`为false时（说人话：有人来看，但是流不存在），通过HTTP API接口触发lalserver从第三方拉流
- 业务方可以在收到`on_sub_stop`事件，并且`has_out_session`为false时（说人话：人走了，没人看了），通过HTTP API接口触发lalserver停止从第三方拉流

### 5 `on_sub_stop`

字段同`on_sub_start`，不再赘述。

### 6 `on_rtmp_connect`

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

### 7 `on_server_start`

字段同HTTP API中`/api/stat/lal_info`中的data字段。

## 其他

```
本文档基于版本：
lal tag: v0.19.0+
tag之后的commit id:
http api version:
http notify version:
```
