# lalserver HTTP API接口

lalserver提供了一些HTTP的API接口，通过这些接口，可以获取lalserver的一些状态，以及控制一些行为。

## 一. 接口列表

接口分为两大类：

- 查询类型的，以`/api/stat`开头
- 控制类型的，以`/api/ctrl`开头

```
1.1. /api/stat/group     // 查询特定group的信息
1.2. /api/stat/all_group // 查询所有group的信息
1.3. /api/stat/lal_info  // 查询服务器信息

2.1. /api/ctrl/start_pull       // 控制服务器从远端拉流至本地
2.2. /api/ctrl/kick_out_session // 强行踢出关闭指定session，比如rtmp pub推流会话、http-flv sub拉流会话等
```

## 二. 名词解释：

<!-- more -->

- group: lal中的group是群组的概念，lal作为流媒体服务器，通过流名称将每1路输入流转发给1~n路输出流，流名称相同的输入输出流被同1个group群组管理。

## 三. 接口规则

1 所有接口的返回结果中，必含的一级参数：

```
{
    "error_code": 0,
    "desp": "succ",
    "data": ...
}
```

`2 error_code`列表：

| `error_code` | desp | 说明|
| - | - | - |
| 0 | succ | 调用成功 |
| 1001 | group not found | group不存在 |
| 1002 | param missing | 必填参数缺失 |
| 1003 | session not found | session不存在 |


## 四. 接口详情

### 1.1 `/api/stat/group`

- 简要描述： 查询指定group的信息
- 请求URL： `http://127.0.0.1:8083/api/stat/group?stream_name=test110`
- 请求方式： `HTTP GET`
- 请求参数：
    - stream_name | 必填项 | 指定group的流名称
- 返回值`error_code`可能取值：
    - 0    group存在，查询成功
    - 1001 group不存在
    - 1002 必填参数缺失

返回示例：

```
{
  "error_code": 0, // 接口返回值，0表示成功
  "desp": "succ",  // 接口返回描述，"succ"表示成功
  "data": {
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
        "read_bitrate": ,                        // 最近5秒读取数据码率
        "write_bitrate": 439                     // 最近5秒发送数据码率
      }
    ],
    "pull": {}, // 该节点从其他节点拉流回源信息，内部字段和上面pub的内部字段相同，不再赘述
    "pushs":[]  // 主动外连转推信息，暂时不提供
  }
}
```

### 1.2 `/api/stat/all_group`

- 简要描述： 查询所有group的信息
- 请求URL示例： `http://127.0.0.1:8083/api/stat/all_group`
- 请求方式： `HTTP GET`
- 请求参数： 无
- 返回值`error_code`可能取值：
    - 0 查询成功

返回示例：

```
{
    "error_code": 0,
    "desp": "succ",
    "data": {
        "groups": [
            ...      // 数组内每个元素的内容格式和/api/stat/group接口中data字段相同，不再赘述
        ]
    }
}
```

### 1.3 `/api/stat/lal_info`

- 简要描述： 查询服务器信息
- 请求URL示例： `http://127.0.0.1:8083/api/stat/lal_info`
- 请求方式： `HTTP GET`
- 请求参数： 无
- 返回值`error_code`可能取值：
    - 0 查询成功

返回示例：

```
{
  "error_code": 0,
  "desp": "succ",
  "data": {
    "server_id": "1",
    "bin_info": "GitTag=v0.17.0. GitCommitLog=bbf850aca2d4f3e55380d44ca9c3a16be60c8d39 ${NewVersion} -> version.go. GitStatus= M CHANGELOG.md | M gen_tag.sh | M pkg/base/version.go. BuildTime=2020.11.21.173812. GoVersion=go version go1.14.2 darwin/amd64. runtime=darwin/amd64.",
    "lal_version": "v0.17.0",               // lal可执行文件版本信息
    "api_version": "v0.1.2",                // HTTP API接口版本信息
    "notify_version": "v0.0.4",             // HTTP Notify版本信息
    "start_time": "2020-11-21 17:34:53.973" // lal进程启动时间
  }
}
```

### 2.1 `/api/ctrl/start_pull`

- 简要描述： 控制服务器主动从远端拉流至本地
- 请求方式： `HTTP POST`

请求示例：

```
$curl -H "Content-Type:application/json" -X POST -d '{"protocol": "rtmp", "addr":"127.0.0.1:19550", "app_name":"live", "stream_name":"test110", "url_param":"token=aaa&p2=bbb"}' http://127.0.0.1:8083/api/ctrl/start_pull
```

请求参数说明：

```
{
  "protocol": "rtmp",            // 必填项，目前只支持 rtmp
  "addr":"127.0.0.1:1935",       // 必填项，远端地址
  "app_name":"live",             // 必填项，app name
  "stream_name":"test110",       // 必填项，流名称
  "url_param":"token=aaa&p2=bbb" // 选填项，url参数
}
```

返回值`error_code`可能取值：

- 0    请求接口成功。注意，返回成功并不保证从远端拉流成功
- 1002 参数错误

### 2.2 `/api/ctrl/kick_out_session`

- 简要描述： 强行踢出关闭指定session，比如rtmp pub推流会话、http-flv sub拉流会话等
- 请求方式： `HTTP POST`

请求示例：

```
$curl -H "Content-Type:application/json" -X POST -d '{"stream_name": "test110", "session_id": "FLVSUB1"}' http://127.0.0.1:8083/api/ctrl/kick_out_session
```

请求参数说明：

```
{
  "stream_name": "test110", // 必填项，流名称
  "session_id": "FLVSUB1"   // 必填项，会话唯一标识
}
```

返回值`error_code`可能取值：

- 0    请求接口成功。指定会话被关闭
- 1001 指定流名称对应的group不存在
- 1003 指定会话不存在

## 其他

HTTP API模块还提供一个API列表页面，地址为`http://127.0.0.1:8083/api/list`

```
本文档基于版本：
lal tag: v0.17.0
tag之后的commit id: 3ca25a22d2b6553ff3ef76199c6ce7a2e3c4e842 [doc] fix demo chart
http api version: v0.1.3
http notify version: v0.0.5

文档最后修改时间：20201213
```

