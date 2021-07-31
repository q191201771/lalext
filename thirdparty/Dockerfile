FROM centos:7
COPY x264-master.tar.bz2 x265-master.zip fdk-aac-2.0.2.tar.gz opus-1.3.1.tar.gz n4.4.tar.gz build.sh mod_ffmpeg_to_support_rtmp_hevc.tgz /thirdparty/
RUN yum install unzip -y \
    && yum install bzip2 -y \
    && yum install zlib zlib-devel -y \
    && yum install gcc -y \
    && yum install gcc+ gcc-c++ -y \
    && yum install make -y \
    && yum install cmake -y \
    && yum install gdb -y \
    && cd /thirdparty && ./build.sh
