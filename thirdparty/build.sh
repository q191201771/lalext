#!/usr/bin/env bash

## 注释 CentOS下安装ffmpeg
##

set -x

## 注释 cmake
##
#yum install cmake -y

## 注释 libx264
##
##      https://www.videolan.org/developers/x264.html
##      https://code.videolan.org/videolan/x264/-/archive/master/x264-master.tar.bz2
##
echo 'libx264...'
tar jxvf x264-master.tar.bz2
cd x264-master
./configure --enable-shared --disable-asm
make -j8 && make install && make clean
cd -

## 注释 libx265
##
##      https://github.com/videolan/x265/archive/refs/heads/master.zip
##
echo 'x265...'
unzip x265-master.zip
cd x265-master/build
cmake ../source
make -j8 && make install && make clean
cd -

## 注释 fdk-aac
##
##      https://www.linuxfromscratch.org/blfs/view/svn/multimedia/fdk-aac.html
##      https://downloads.sourceforge.net/opencore-amr/fdk-aac-2.0.2.tar.gz
##
echo 'fdk-aac...'
tar zxvf fdk-aac-2.0.2.tar.gz
cd fdk-aac-2.0.2
./configure
make -j8 && make install && make clean
cd -

## 注释 opus
##
##      https://opus-codec.org/downloads/
##      https://archive.mozilla.org/pub/opus/opus-1.3.1.tar.gz
##
echo 'opus...'
tar zxvf opus-1.3.1.tar.gz
cd opus-1.3.1
./configure
make -j8 && make install && make clean
cd -

## 注释 ffmpeg
##
##      https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n4.4.tar.gz
##
##      --extra-cflags="-fno-stack-check" https://pengrl.com/p/20042/
##
echo 'ffmpeg...'
tar zxvf n4.4.tar.gz
tar zxvf mod_ffmpeg_to_support_rtmp_hevc.tgz
cp mod_ffmpeg_to_support_rtmp_hevc/libavformat/flv.h FFmpeg-n4.4/libavformat/flv.h
cp mod_ffmpeg_to_support_rtmp_hevc/libavformat/flvdec.c FFmpeg-n4.4/libavformat/flvdec.c
cp mod_ffmpeg_to_support_rtmp_hevc/libavformat/flvenc.c FFmpeg-n4.4/libavformat/flvenc.c
cd FFmpeg-n4.4
echo 'ffmpeg configure...'
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
./configure --enable-pic --extra-cflags="-fno-stack-check" --disable-x86asm \
            --enable-gpl --enable-nonfree --enable-libx264 --enable-libx265 --enable-libfdk-aac --enable-libopus
make -j8 && make install && make clean
cd -

## 注释 验证ffmpeg以及库是否能正常使用
##
#export LD_LIBRARY_PATH=/usr/local/lib/
#cd FFmpeg-n4.4
#./ffmpeg
#cd doc/example
#gcc filtering_video.c -lavfilter -lavformat -lavdevice -lavcodec -lavutil -lpostproc -lswresample -lswscale -lm -lz -lx264 -lx265 -lfdk-aac -lopus -lpthread
#./a.out
