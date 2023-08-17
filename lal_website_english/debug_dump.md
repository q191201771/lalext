# debug dump抓取lal流调试

lal支持将输入流的数据dump保存成二进制文件，事后通过该dump文件，将流再次输入lal进行数据回放。

出现问题时，你可以把数据dump下来，提供给lal的作者，让他帮忙分析。

以下是各协议对应的dump方法：

#### ▦ GB28181 

通过lalserver的HTTP-API的`/api/ctrl/start_rtp_pub`接口中的`debug_dump_packet`参数

> 具体见： https://pengrl.com/lal/#/HTTPAPI?id=_24-apictrlstart_rtp_pub

#### ▦ RTSP

RTSP有两种手段：

##### ✒ lalserver

第一种是通过lalserver的HTTP-API的`/api/ctrl/start_relay_pull`接口中的`debug_dump_packet`参数

> 具体见： https://pengrl.com/lal/#/HTTPAPI?id=_21-apictrlstart_relay_pull

##### ✒ demo pullrtsp

第二种方法是使用`app/demo/pullrtsp`这个demo直接拉取rtsp流

使用方式如下：

```
./bin/pullrtsp -i rtsp://localhost:5544/live/test110 -o outpullrtsp.flv -t 0 -d outpullrtsp.laldump
```

拉取rtsp流后，将生成的outpullrtsp.flv，outpullrtsp.laldump，pullrtsp.log三个文件。

#### ▦ customize_pub插件自定义流

调用`CustomizePubSessionContext::WithCustomizePubSessionContextOption`方法，将其中的`DebugDumpPacket`参数设置为保存文件的名称，比如"./dump/test110.laldump", "/tmp/test110.laldump"。

yoko, 202212
