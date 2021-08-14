#!/usr/bin/env bash

docker build -t lalcc_demo .
docker run -i -t lalcc_demo /output/bin/pullrtmp2es $RTMP_URL
