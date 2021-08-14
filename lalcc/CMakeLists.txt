cmake_minimum_required(VERSION 2.8)
set(CMAKE_VERBOSE_MAKEFILE ON)
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

project(lalcc_demo C CXX)

set(CMAKE_CXX_FLAGS "-g -pthread -std=c++0x -pipe -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wunused-parameter -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -Wcast-align")

message(STATUS ${CMAKE_BUILD_TYPE})
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

set(ROOT_DIR ${CMAKE_SOURCE_DIR})
message(${ROOT_DIR})
include_directories(/usr/local/include ${ROOT_DIR}/include)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

link_directories(/usr/local/lib)

#add_executable(pullrtmp2pushrtmp demo/pullrtmp2pushrtmp/pullrtmp2pushrtmp.cc)
#target_link_libraries(pullrtmp2pushrtmp avfilter avformat avdevice avcodec avutil postproc swresample swscale x264 x265 fdk-aac opus m z dl pthread rt)

add_executable(pullrtmp2es demo/pullrtmp2es/pullrtmp2es.cc)
target_link_libraries(pullrtmp2es avfilter avformat avdevice avcodec avutil postproc swresample swscale x264 x265 fdk-aac opus m z dl pthread rt)
