#!/usr/bin/env bash

docker build -t lalcc_demo .
docker run -i -t lalcc_demo /output/bin/lalcc_demo
