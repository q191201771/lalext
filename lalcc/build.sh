#!/usr/bin/env bash

set -x

rm -rf ./output
mkdir ./output
cd ./output && cmake .. && make && pwd
