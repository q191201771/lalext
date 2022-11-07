### chunk stream id

#### 设计思想

按rtmp的设计思想，是可以在一条链路上传输多路流，比如最常见的一路音频流和一路视频流，音频流和视频流可独立打开关闭。
那么当某一路流中的message特别大时（比如视频的关键帧），可能会导致音频流的message需要等待这个特别大的message传输完毕后才能传输。
rtmp为了解决这个问题，选择将对应用层有意义的message切割成chunk，那么传输时就不需要一个message接一个message传输了，而可以一个chunk接一个chunk传输。
（注意，chunk必须是完整的，不能发送chunk1的前半部分再发chunk2然后再发送chunk1的后半部分）
接收端将`chunk stream id`相同的chunk组合成完整的message再返回给应用层。

下面是一个简单的例子

```
# 假设Video message 1很大

# 最原始的多路流传输方式，Audio Message 2只能等待Video message 1传输完毕后才能传输
| Audio message 1 | Video message 1 | Audio Message 2 | Video message2 |

# 将Video Message 1切割成两个chunk（这里为了简化，其实可能不止两个chunk）
# 这样，Audio Message 2就不需要等待整个Video message 1发送完毕再发送了
# 另外，其他的message也是chunk的形式了
| Audio message 1 | first chunk of Video meesage 1 | Audio Message 2 | last chunk of Video message 2 | Video message 2 |

# 以下这种是错误的，chunk必须保持完整
| chunk1的前半部分 | chunk2的前半部分 | chunk1的后半部分 | chunk2的后半部分 |
```

#### 思考

那我们来思考一下，按现在常见的rtmp over tcp的方案，当应用层有一个message需要传输时，我们会将它切割成chunk，然后发送切割出的所有chunk，然后再循环处理下一个message。

事实上，一个message的chunk是被连续发送的，并不会插入其他chunk。此基础上基于tcp的流式顺序性，是无法达到上面大message不阻塞其他stream的message的设计。

那么理论上rtmp的这个设计思路可不可行呢？是可行的，比如发送队列不使用FIFO的大buffer，而是以chunk组成的更细粒度的管理。或者使用UDP传输，一般都会按MTU切割应用层的message，基本和chunk的概念类似。

以上其实就是Head-of-Line blocking对头阻塞问题，QUIC HTTP/3使用了类似的思路处理这个问题。

#### 实践

下面是一些常见的rtmp项目使用chunk stream id的情况

#### obs推流

```
csid 2 用于传输 SetChunkSize。
csid 3 用于传输信令，比如 connect | releaseStream | FCPublish | createStream | publish
csid 4 用于传输 metaData + 音频数据 + 视频数据。
```

#### ffmpeg推流

```
csid 2 SetChunkSize
csid 3 connect | releaseStream | FCPublish | createStream
csid 8 publish
csid 4 metaData + 视频数据
csid 6 音频数据
csid 3 FCUnpublish | deleteStream
```

注意，ffmpeg推流时，是先发的connect，再发SetChunkSize，接收端在没有收到SetChunkSize时，应该假定对端的chunk size为128。

#### 从nginx rtmp module拉流

```
csid 2 Window ACk Size | SetPeerBandwidth | SetChunkSize
csid 3 onBWDone | _result for Connect | _result for createStream
csid 5 onStatus for Play
csid 5 metaData e.g. len 24
csid 2 user control message
csid 5 metaData e.g. len 699
csid 7 视频数据
csid 6 音频数据
```

可以看到，obs推流的音频和视频数据共用一个csid，ffmpeg则是两个csid，nginx rtmp module则是metaData也单独使用一个csid。

chunk stream id如何获取，message如何切割成chunk，chunk如何重组成message可以参见 [lal](https://github.com/q191201771/lal) (https://github.com/q191201771/lal)的实现。

### message stream id

#### obs推流

```
msid 0 信令
msid 1 publish信令 | 音视频数据
```

#### ffmpeg推流

和obs一样

#### 从nginx rtmp module拉流

```
msid 0 信令
msid 1 onStatus of play信令 | 音视频数据
```

可以看到，三个项目都是使用msid 1传输音视频数据。

lal中，`_result for createStream`的msid使用1，publish 和 play 使用`_result`中返回msid

### transaction id

command类型message中的一个字段。

尽管文档中将大部分message都定义为0了，比如:

```
connect tid: 1
onStatus: 0
play: 0
deleteStream: 0
publish: 0
```

但是各rtmp软件的具体实现却不是这样，比如：

#### obs推流

```
connect tid: 1
releaseStream: 2
FCPublish: 3
createStream: 4
publish: 5
FCUnpublish: 6
deleteStream: 7
```

可以看到是自增的过程。
测试时服务端使用lal，lal是按照接收到的tid原样回复message。

#### ffmpeg推流

结果和obs推流一样

#### 从nginx rtmp module拉流

```
onBWDone tid: 0
_result of connect: 1
_result of createStream: 2
onStatus of Play: 0
```

测试时客户端使用lal，lal按递增的方式处理tid：

```
connect tid: 1
createStream: 2
play or publish: 3
```

### message type id

这个是最简单的，标明应用层的message是什么类型，比如音频(8)、视频(9)、metadata(18)、特定信令(比如1是SetChunkSize)、Command类型的信令(20)等。

### TODO

现在rtmp的使用场景基本上都是一个Connection对应一次推流或者拉流。
其实rtmp在flash中使用时，一个Connection对象上是可以打开、关闭多个Stream，这才是最贴合rtmp语义的用法。由于我手上没有合适的环境，暂时没有测试。

原创不易，转载请注明文章出自开源流媒体服务器[lal](https://github.com/q191201771/lal)，Github：[https://github.com/q191201771/lal](https://github.com/q191201771/lal)  官方文档：[https://pengrl.com/lal](https://pengrl.com/lal)  

yoko, 201908
