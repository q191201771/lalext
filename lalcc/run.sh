#!/usr/bin/env bash

# 运行docker
#docker build -t lalcc_demo .
#docker run -i -t lalcc_demo ./output/bin/pullrtmp2pushrtmp $RTMP_URL $RTMP_URL2

# 运行本地
./build.sh
./output/bin/pullrtmp2pushrtmp $RTMP_URL $RTMP_URL2
