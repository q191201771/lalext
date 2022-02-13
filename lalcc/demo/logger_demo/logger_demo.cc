// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

#include "liblalcc/logger.hpp"
#include "chef_base/chef_stuff_op.hpp"

// 32 * 1000000           31M 1002ms
// 128 * 400 * 2 * 8 * 50 39M 882ms
int logSize = 128;  // 单条日志大小
int logCount = 400 * 2 * 8 * 50; // 日志数量（单个线程）
//int threadNum = 1;   // 日志线程数（上层调用并发数，不是底层日志线程的数量）

int main(int argc, char **argv) {
  unstable::LoggerOption option;
  option.Level_ = unstable::LevelDebug;
  option.Filename_ = "log/logger_demo13.log";
  option.IsToStdout_ = false;
  option.IsRotateDaily_ = true;
  option.ShortFileFlag_ = false;
  option.TimestampFlag_ = false;
  option.TimestampWithMsFlag_ = false;
  option.LevelFlag_ = false;
  auto l = unstable::NewLogger(option);

  char *buf = new char[logSize+1];
  memset(buf, 'a', logSize);
  buf[logSize] = '\0';

  auto b = chef::stuff_op::tick_msec();

  for (int i =0; i < logCount; i++) {
    CHEF_LOGGER_INFO(l, "%s", buf);
  }

  auto e = chef::stuff_op::tick_msec();
  printf("cost:%llums", e - b);

  return 0;
}