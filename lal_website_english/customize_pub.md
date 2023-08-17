# 二次开发-pub接入自定义流

### ▦ 场景举例

我们设想一个**需求场景**，业务方原有系统中，有自定义协议的流，现在想把他们输入到lalserver中，**从而使用lalserver的功能**，比如录制，或者（使用lalserver所支持的协议）从lalserver拉流播放等等。  
一个现行的方法，业务方可以在自己的系统中，把流转换为lalserver支持的标准协议流，使用网络连接推送至lalserver。  
另外一个方法，是使用lalserver提供的customize pub功能。此时，业务方的代码将在lalserver进程中执行。  
注意，两种方法，**流进入lalserver后，效果是一样的**。  
区别只是**架构不同**。也即是否在lalserver中集成业务方的代码。  

换言之，第一种方法，lalserver更为独立。第二种方法，lalserver与业务方的系统结合的更为紧密，某些情况下可减少一些中间流。

### ▦ What

lalserver在**代码层面**提供了接口，业务方可调用lalserver的函数，向lalserver添加或者删除自定义流，并且输入音视频数据。

啥意思呢，说人话就是，以前你把lalserver启动起来，流进入到lalserver有两种方式：

1. 使用客户端将流推送到lalserver上（lalserver端一般称之为pub）
2. 使用配置文件或者HTTP API通知lalserver去其他地方把流拉回来（lalserver一般称之为relay pull）

现在呢，多了一种方式：在lalserver之上**写定制化的代码**，将流数据（处于同一个进程中的内存块）直接传到lalserver中。

> 该功能在下文和lalserver中称为customize pub。

### ▦ How

talk is cheap，直接上一个完整的例子。  
[lal/app/demo/customize_lalserver](https://github.com/q191201771/lal/blob/master/app/demo/customize_lalserver/customize_lalserver.go)中，借助读取一个h264 es流文件和一个aac es流文件，演示如何使用customize pub。

注意，从使用场景来说，**lalserver并不关心业务方的流的来源，也不关心流的原始格式。**

在业务方的代码里，流来源可以是文件，也可以是网络。原始格式可以是es，也可以是任意其他格式。  
业务方只要**根据自身业务逻辑选择时机**调用lalserver的接口创建流，并**将流转换成lalserver所要求的格式**传入即可。

其他的就看demo的代码吧。代码很少，并且有详细的注释。  
唯一需要提醒的是，整个demo里的代码你都应该把它看成是业务方的代码。目的是让你学会怎么和lalserver交互，不是学会怎么用这个demo。。

测试用音频和视频文件可以去 https://github.com/q191201771/lalext 这个github repo的avfile目录下载。

this note updated at 2210 by yoko
