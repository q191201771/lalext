#!/usr/bin/env bash

## 注释 MacOS或CentOS下安装ffmpeg
##
##      注意：
##      1. 执行需要root权限，比如 sudo ./build.sh
##      2. 依赖 cmake
##      3. 依赖 g++，比如 yum install gcc-c++.x86_64
##      4. gcc/g++版本 https://pengrl.com/p/42585/
##      5. 依赖 automake，比如 brew install automake
##      6. 依赖 autoconf，比如 brew install autoconf@2.69
##      7. 依赖 pkg-config，比如 brew install pkg-config
##      https://www.pengrl.com/p/20043/
##      brew install xquartz sdl glfw3 glew
##
## centos 安装依赖：
## yum -y install cmake unzip bzip2 bzip2-libs bzip2-devel
## yum install centos-release-scl -y
## yum install devtoolset-9 -y
## scl enable devtoolset-9 bash
##
## macos m1没有编译出ffplay，我自己手动编译的
##      gcc cmdutils.c ffplay.c -I.. -I/opt/homebrew/include/SDL2 -lavdevice -lavfilter -lavformat -lavcodec -lpostproc -lswresample -lswscale -lavutil /opt/homebrew/lib/libSDL2.dylib
##
## 注释 nasm
##
##      http://www.nasm.us/pub/nasm/releasebuilds/2.13.03/nasm-2.13.03.tar.xz
##      https://blog.csdn.net/weixin_43328157/article/details/109046492
##
## 注释 libx264
##
##      https://www.videolan.org/developers/x264.html
##      https://code.videolan.org/videolan/x264/-/archive/master/x264-master.tar.bz2
##
## 注释 libx265
##
##      https://github.com/videolan/x265/archive/refs/heads/master.zip
##
## 注释 fdk-aac
##
##      https://www.linuxfromscratch.org/blfs/view/svn/multimedia/fdk-aac.html
##      https://downloads.sourceforge.net/opencore-amr/fdk-aac-2.0.2.tar.gz
##
## 注释 opus
##
##      https://opus-codec.org/downloads/
##      https://archive.mozilla.org/pub/opus/opus-1.3.1.tar.gz
##
## 注释 libass
##
##      https://github.com/libass/libass/releases/tag/0.17.1
##      https://github.com/libass/libass/releases/download/0.17.1/libass-0.17.1.tar.gz
##
## 注释 libfreetype
##
##      http://mirror.ossplanet.net/nongnu/freetype/
##      http://mirror.ossplanet.net/nongnu/freetype/freetype-2.13.0.tar.gz
##
## 注释 ffmpeg
##
##      https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n4.4.tar.gz
##
##      --extra-cflags="-fno-stack-check" https://pengrl.com/p/20042/
##
##
## 注释 注意，--enable-debug 这一行的选项打开用于编译debug版本
##
## 注释 验证ffmpeg以及库是否能正常使用
##
## 注释 没有安装libfontconfig，而是直接使用我macOS上之前已经有的

set -x

export ACCEPT_INFERIOR_RM_PROGRAM=yes

PREFIX=/usr/local

echo 'nasm...'
tar -xvf nasm-2.13.03.tar
cd nasm-2.13.03
./configure  --prefix=${PREFIX}
make -j8 && make install && make clean
cd -

## 注释 --disable-asm configure参数
echo 'libx264...'
tar jxvf x264-master.tar.bz2
cd x264-master
./configure --prefix=${PREFIX} --enable-shared --enable-pic --extra-cflags="-fno-stack-check"
make -j8 && make install && make clean
cd -

echo 'x265...'
unzip x265-master.zip
cd x265-master/build
cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} ../source
make -j8 && make install && make clean
cd -

echo 'fdk-aac...'
tar zxvf fdk-aac-2.0.2.tar.gz
cd fdk-aac-2.0.2
./configure --prefix=${PREFIX}
make -j8 && make install && make clean
cd -

echo 'opus...'
tar zxvf opus-1.3.1.tar.gz
cd opus-1.3.1
./configure --prefix=${PREFIX}
make -j8 && make install && make clean
cd -

echo 'libfreetype...'
tar zxvf freetype-2.13.0.tar.gz
cd freetype-2.13.0
./configure --prefix=${PREFIX}
make -j8 && make install && make clean
cd -

# libfontconfig

#echo 'libass...'
#tar zxvf libass-0.17.1.tar.gz
#cd libass-0.17.1/
#./configure --prefix=${PREFIX}
#make -j8 && make install && make clean
#cd -

echo 'ffmpeg...'
tar zxvf n4.4.tar.gz
tar zxvf mod_ffmpeg_to_support_rtmp_hevc.tgz
cp mod_ffmpeg_to_support_rtmp_hevc/libavformat/flv.h FFmpeg-n4.4/libavformat/flv.h
cp mod_ffmpeg_to_support_rtmp_hevc/libavformat/flvdec.c FFmpeg-n4.4/libavformat/flvdec.c
cp mod_ffmpeg_to_support_rtmp_hevc/libavformat/flvenc.c FFmpeg-n4.4/libavformat/flvenc.c
cd FFmpeg-n4.4
echo 'ffmpeg configure...'
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
./configure --prefix=${PREFIX} --extra-cflags="-I${PREFIX}/include -L${PREFIX}/lib -fno-stack-check -g" \
            --enable-pic \
            --enable-shared --disable-static \
            --enable-gpl --enable-nonfree \
            --enable-debug=3 --disable-optimizations --disable-stripping \
            --disable-x86asm --disable-asm \
            --enable-libx264 --enable-libx265 --enable-libfdk-aac --enable-libopus --enable-libfreetype --enable-libfontconfig

# 注释 汇编优化相关的configure参数，加上会影响性能 --disable-x86asm --disable-asm
#  --enable-libass
# --extra-cflags="-I${PREFIX}/include -I/opt/homebrew/include -L${PREFIX}/lib -L/opt/homebrew/lib"
# --extra-libs="-lpthread -lm" \

make -j8 && make install
cd -

echo 'try build ffmpeg example...'
export LD_LIBRARY_PATH=/usr/local/lib/
cd FFmpeg-n4.4
./ffmpeg
cd doc/examples
gcc filtering_video.c -lavfilter -lavformat -lavdevice -lavcodec -lavutil -lpostproc -lswresample -lswscale -lm -lz -lx264 -lx265 -lfdk-aac -lopus -lpthread
./a.out

cd ../../fftools
gcc cmdutils.c ffplay.c -I.. -I/opt/homebrew/include/SDL2 -lavdevice -lavfilter -lavformat -lavcodec -lpostproc -lswresample -lswscale -lavutil /opt/homebrew/lib/libSDL2.dylib -o ffplay
mv ffplay /usr/local/bin
