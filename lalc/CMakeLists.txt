cmake_minimum_required(VERSION 2.8)
set(CMAKE_VERBOSE_MAKEFILE ON)
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

project(lalc C)

#set(CMAKE_CXX_FLAGS "-g -pthread -std=c++0x -pipe -fPIC")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wunused-parameter -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings -Wcast-align")

message(STATUS ${CMAKE_BUILD_TYPE})
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

set(ROOT_DIR ${CMAKE_SOURCE_DIR})
message(${ROOT_DIR})
include_directories(/usr/local/include ${ROOT_DIR})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../output)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../output)

link_directories(/usr/local/lib)

if(APPLE)
  message(STATUS "os: APPLE")
  #set(COMMON_LIBS pthread)
elseif(UNIX)
  message(STATUS "os: UNIX")
  set(COMMON_LIBS pthread rt)
endif()

add_library(lalc liblalc/lalc.c)

add_executable(lalc_demo demo/c/lalc_demo.c)
target_link_libraries(lalc_demo lalc ${COMMON_LIBS} avfilter avformat avdevice avcodec avutil postproc swresample swscale x264 x265 fdk-aac opus m z dl)
