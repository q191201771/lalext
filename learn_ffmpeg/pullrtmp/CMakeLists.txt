cmake_minimum_required(VERSION 2.8)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(pullrtmp C CXX)

set(CMAKE_CXX_FLAGS "-g -pthread -std=c++0x -pipe -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wunused-parameter -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -Wcast-align")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

include_directories(/usr/local/include)

set(ROOT_DIR ${CMAKE_SOURCE_DIR})
message(${ROOT_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

link_directories(/usr/local/lib)
add_executable(pullrtmp pullrtmp.cc)
target_link_libraries(pullrtmp avfilter avformat avdevice avcodec avutil postproc swresample swscale x264 x265 fdk-aac opus m z dl pthread rt)
