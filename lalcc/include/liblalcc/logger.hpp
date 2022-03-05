// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _LALCC_LOGGER_HPP_
#define _LALCC_LOGGER_HPP_

// logger.hpp
//
// 简单的日志实现
//

// TODO(chef):
//   - 移入libchef中
//   - prefix
//   - pid flag
//   - logger对象释放
//   - 异步日志

#include <utility>
#include <queue>
#include <fcntl.h>

#include "chef_base/chef_buffer.hpp"
#include "chef_base/chef_env.hpp"
#include "chef_base/chef_filepath_op.hpp"
#include "chef_base/chef_task_thread.hpp"

namespace unstable {
  class ILogger;
  typedef chef::shared_ptr<ILogger> ILoggerPtr;

  enum Level {
    LevelTrace = 0,
    LevelDebug = 1,
    LevelInfo = 2,
    LevelWarn = 3,
    LevelError = 4,
    LevelFatal = 5,
    LevelPanic = 6,
    LevelLogNothing = 7,
  };

  struct LoggerOption {
//    LoggerOption() : Level_(LevelDebug), IsToStdout_(true), IsRotateDaily_(false), ShortFileFlag_(true), TimestampFlag_(true), TimestampWithMsFlag_(true), LevelFlag_(true) {}

    Level Level_ = LevelDebug;

    std::string Filename_;
    bool IsToStdout_ = true;

    bool IsRotateDaily_ = false;

    bool ShortFileFlag_ = true;
    bool TimestampFlag_ = true;
    bool TimestampWithMsFlag_ = true;
    bool LevelFlag_ = true;

    int BufInitCap_ = 4096;
    int BufShrinkCap_ = 16 * 1024 * 1024;

    // TODO(chef): AssertBehavior
  };

  // InitDefaultLogger 初始化全局logger
  //
  // 注意，如果需要初始化，则调用方保证只调用一次，并且调用发生在所有`DefaultLogger()`调用之前
  //
  void InitDefaultLogger(const LoggerOption &loggerOption=LoggerOption());

  // TODO(chef): SetDefaultLogger

  ILoggerPtr DefaultLogger();

  ILoggerPtr NewLogger(const LoggerOption &loggerOption=LoggerOption());

  class ILogger {
    public:
      virtual void Infof(const char *srcfile, int srcline, const char *format, ...) = 0;

      virtual ~ILogger() = default;
  };


#define CHEF_LOGGER_INFO(logger, ...) (logger)->Infof(__FILE__, __LINE__, __VA_ARGS__);

#define CHEF_DEFAULT_LOGGER_INFO(...) CHEF_LOGGER_INFO(unstable::DefaultLogger(), __VA_ARGS__);

}
// --------------------------------------------------------------------------------------------------------------------

typedef chef::basic_buffer<char, std::size_t> Buffer;

//typedef chef::shared_ptr<Buffer> BufferPtr;
//
//class BufferPool {
//public:
//  BufferPool(std::size_t buf_init_cap, std::size_t buf_shrink_cap) : buf_init_cap_(buf_init_cap), buf_shrink_cap_(buf_shrink_cap) {}
//
//  BufferPtr Acquire() {
//    chef::lock_guard<chef::mutex> guard(mu_);
//    if (queue_.empty()) {
//      return std::make_shared<Buffer>();
//    }
//    auto ret = queue_.front();
//    queue_.pop();
//    return ret;
//  }
//
//  void Release(BufferPtr buf) {
//    chef::lock_guard<chef::mutex> guard(mu_);
//    queue_.push(buf);
//  }
//private:
//  std::size_t buf_init_cap_;
//  std::size_t buf_shrink_cap_;
//
//  chef::mutex mu_;
//  std::queue<BufferPtr> queue_;
//};

// --------------------------------------------------------------------------------------------------------------------

namespace unstable {

  class Logger : public ILogger, chef::enable_shared_from_this<Logger> {
    public:
      explicit Logger(const LoggerOption &option);
      ~Logger() override;

      void Infof(const char *srcfile, int srcline, const char *format, ...) override;

    private:
      void writeTime(Buffer &buf,  std::chrono::system_clock::time_point now, bool withMs);
      std::tm localtime(std::chrono::system_clock::time_point &tp);
      std::string Dir(std::string path);
      std::string levelToString(Level level);
      std::string levelToColorString(Level level);
      void backend(Buffer &buf);

    private:
      LoggerOption option_;

      chef::mutex mu_;
      FILE *fp_ = nullptr;
      FILE *console_ = nullptr;
      Buffer buf_;
      std::chrono::system_clock::time_point currRoundTime_;

  };

  inline Logger::Logger(const LoggerOption &option) : option_(option), buf_(option.BufInitCap_, option.BufShrinkCap_), currRoundTime_(std::chrono::system_clock::now()) {
    // TODO(chef): validate

    if (!option.Filename_.empty()) {
      auto dir = Dir(option.Filename_);
      if (!dir.empty()) {
        chef::filepath_op::mkdir_recursive(dir);
      }
      fp_ = fopen(option.Filename_.c_str(), "a");
    }
    if (option.IsToStdout_) {
      console_ = stdout;
    }
  }

  inline Logger::~Logger() {
  }

  inline void Logger::Infof(const char *srcfile, int srcline, const char *format, ...) {
    // TODO(chef): level check

    chef::lock_guard<chef::mutex> guard(mu_);

    Buffer &buf = buf_;

    // frontend
    // 构造日志
    {
      buf.clear();

      auto now = std::chrono::system_clock::now();

      if (option_.TimestampFlag_) {
        writeTime(buf, now, option_.TimestampWithMsFlag_);
      }

      if (option_.LevelFlag_) {
        std::string levelString;
        if (console_) {
          levelString = levelToColorString(option_.Level_);
        } else {
          levelString = levelToString(option_.Level_);
        }
        buf.append(levelString.c_str(), levelString.length());
      }

      va_list ap;
      va_start(ap, format);
      buf.reserve(4096);
      int len = vsnprintf(buf.write_pos(), 4096, format, ap);
      if (len > 4096) {
        buf.reserve(len);
        vsnprintf(buf.write_pos(), len, format, ap);
      }
      va_end(ap);
      buf.seek_write_pos(len);

      if (option_.ShortFileFlag_) {
        std::string srcfileWithoutPath(srcfile);
        std::size_t pos = srcfileWithoutPath.rfind('/');
        if (pos != std::string::npos) {
          srcfileWithoutPath = srcfileWithoutPath.substr(pos + 1);
        }
        buf.reserve(128);
        len = snprintf(buf.write_pos(), 128, " - %s:%d", srcfileWithoutPath.c_str(), srcline);
        if (len > 128) {
          len = 128;
        }
        buf.seek_write_pos(len);
      }

      if (buf.readable_size() == 0 || buf.read_pos()[buf.readable_size()-1] != '\n') {
        buf.append("\n", 1);
      }
    }

    backend(buf);
  }

  inline void Logger::backend(Buffer &buf) {
//    printf("%d\n", this->thread_->num_of_undone_task());
    auto now = std::chrono::system_clock::now();

    if (console_) {
      fwrite(buf.read_pos(), buf.readable_size(), 1, console_);
      if (option_.Level_ == LevelFatal || option_.Level_ == LevelPanic) {
        fflush(console_);
      }
    }

    if (fp_) {
      if (option_.IsRotateDaily_) {
        auto nowlt = localtime(now);
        auto crtlt = localtime(currRoundTime_);
        if (nowlt.tm_mday != crtlt.tm_mday) {
          char backupName[1024] = {0};
          snprintf(backupName, 1024, "%s.%d%d%d", option_.Filename_.c_str(), nowlt.tm_year+1900, nowlt.tm_mon+1, nowlt.tm_mday);
          chef::filepath_op::rename(option_.Filename_, backupName);
          fclose(fp_);
          fp_ = fopen(option_.Filename_.c_str(), "a");
          currRoundTime_ = now;
        }
      }

      fwrite(buf.read_pos(), buf.readable_size(), 1, fp_);
      if (option_.Level_ == LevelFatal || option_.Level_ == LevelPanic) {
        fflush(fp_);
      }
    }
  }

  inline void Logger::writeTime(Buffer &buf, std::chrono::system_clock::time_point now, bool withMs) {
    // TODO(chef):
    //   - 同一秒可以优化
    //   - 和其他使用时间的逻辑一起，减少重复调用

    auto stm = localtime(now);

    buf.reserve(128);
    int n = 0;
    if (withMs) {
      using std::chrono::duration_cast;
      using std::chrono::seconds;
      auto duration = now.time_since_epoch();
      auto secs = duration_cast<seconds>(duration);
      auto milli = static_cast<uint32_t>((duration_cast<std::chrono::milliseconds>(duration) - duration_cast<std::chrono::milliseconds>(secs)).count());

      n = snprintf(buf.write_pos(), 128, "%d/%02d/%02d %02d:%02d:%02d.%03d ", stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, milli);
    } else {
      n = snprintf(buf.write_pos(), 128, "%d/%02d/%02d %02d:%02d:%02d ", stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec);
    }
    buf.seek_write_pos(n);
  }

  inline std::tm Logger::localtime(std::chrono::system_clock::time_point &tp) {
    auto time_tt = std::chrono::system_clock::to_time_t(tp);
    std::tm stm{};
    ::localtime_r(&time_tt, &stm);
    return stm;
  }

  inline std::string Logger::Dir(std::string path) {
    // TODO(chef): move to libchef

    int i = path.size();
    if (i == 0) {
      return "";
    }
    i--;

    for (; i >= 0; i--) {
      if (path[i] == '/') {
        return path.substr(0, i);
      }
    }
    return "";
  }

  inline std::string Logger::levelToString(Level level) {
    switch (level) {
      case LevelTrace:
        return "TRACE ";
      case LevelDebug:
        return "DEBUG ";
      case LevelInfo:
        return " INFO ";
      case LevelWarn:
        return " WARN ";
      case LevelError:
        return "ERROR ";
      case LevelFatal:
        return "FATAL ";
      case LevelPanic:
        return "PANIC ";
      default:
        return "";
    }
  }

  inline std::string Logger::levelToColorString(Level level) {
    switch (level) {
      case LevelTrace:
        return "\033[22;32mTRACE\033[0m ";
      case LevelDebug:
        return "\033[22;34mDEBUG\033[0m ";
      case LevelInfo:
        return "\033[22;36m INFO\033[0m ";
      case LevelWarn:
        return "\033[22;33m WARN\033[0m ";
      case LevelError:
        return "\033[22;31mERROR\033[0m ";
      case LevelFatal:
        return "\033[22;31mFATAL\033[0m ";
      case LevelPanic:
        return "\033[22;31mPANIC\033[0m ";
      default:
        return "";
    }
  }


// --------------------------------------------------------------------------------------------------------------------

  template<class Dummy>
  struct DefaultLoggerStatic {
    static ILoggerPtr defaultLogger_;
    static chef::mutex defaultLoggerMu_;
  };

  template<class Dummy>
  ILoggerPtr DefaultLoggerStatic<Dummy>::defaultLogger_;

  template<class Dummy>
  chef::mutex DefaultLoggerStatic<Dummy>::defaultLoggerMu_;

  void InitDefaultLogger(const LoggerOption &loggerOption) {
    chef::lock_guard<chef::mutex> guard(DefaultLoggerStatic<void>::defaultLoggerMu_);
    DefaultLoggerStatic<void>::defaultLogger_ = NewLogger(loggerOption);
  }

  ILoggerPtr DefaultLogger() {
    if (DefaultLoggerStatic<void>::defaultLogger_ == nullptr) {
      DefaultLoggerStatic<void>::defaultLoggerMu_.lock();
      if (DefaultLoggerStatic<void>::defaultLogger_ == nullptr) {
        DefaultLoggerStatic<void>::defaultLogger_ = NewLogger();
      }
    }
    return DefaultLoggerStatic<void>::defaultLogger_;
  }

  ILoggerPtr NewLogger(const LoggerOption &option) {
    return chef::make_shared<Logger>(option);
  }

}

#endif
