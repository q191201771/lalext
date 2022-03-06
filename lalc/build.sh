#!/usr/bin/env bash

mkdir liblalc_output
cd liblalc_output && cmake .. && make
cd .. && rm -rf liblalc_output

cd demo/cgo && go build -o ../../output/cgo_lalc_demo
