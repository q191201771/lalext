// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _LALCC_LOGGER_HPP_
#define _LALCC_LOGGER_HPP_

// TODO(chef): 移入libchef中

#include "chef_base/chef_buffer.hpp"
#include "chef_base/chef_env.hpp"

namespace unstable {
  class ILogger;
  typedef chef::shared_ptr<ILogger> ILoggerPtr;

  class ILogger {
    public:
      virtual void Infof(const char *srcfile, int srcline, const char *format, ...) = 0;
  };

  ILoggerPtr DefaultLogger();
  ILoggerPtr NewLogger();

#define CHEF_LOGGER_INFO(logger, ...) (logger)->Infof(__FILE__, __LINE__, __VA_ARGS__);

#define CHEF_DEFAULT_LOGGER_INFO(...) CHEF_LOGGER_INFO(unstable::DefaultLogger(), __VA_ARGS__);

}

// --------------------------------------------------------------------------------------------------------------------

namespace unstable {

  class Logger : public ILogger {
    public:
      void Infof(const char *srcfile, int srcline, const char *format, ...) {
        chef::lock_guard<chef::mutex> guard(mu_);

        chef::basic_buffer<char, std::size_t> buf(16384);

        buf.append("2021/08/06 11:11:11.xxx ", 24);

        va_list ap;
        va_start(ap, format);
        //buf.reserve(12288);
        int len = vsnprintf(buf.write_pos(), 12288, format, ap);
        if (len > 12288) {
          len = 12288;
        }
        va_end(ap);
        buf.seek_write_pos(len);

        std::string srcfileWithoutPath(srcfile);
        std::size_t pos = srcfileWithoutPath.rfind('/');
        if (pos != std::string::npos) {
            srcfileWithoutPath = srcfileWithoutPath.substr(pos + 1);
        }
        //buf.reserve(128);
        len = snprintf(buf.write_pos(), 128, " - %s:%d", srcfileWithoutPath.c_str(), srcline);
        if (len > 128) {
            len = 128;
        }
        buf.seek_write_pos(len);

        buf.append("\n\0", 2);

        printf("%s", buf.read_pos());
      }

    private:
      chef::mutex mu_;
  };

  ILoggerPtr DefaultLogger() {
    static ILoggerPtr dl = NewLogger();
    return dl;
  }

  ILoggerPtr NewLogger() {
    return chef::make_shared<Logger>();
  }

}

#endif
