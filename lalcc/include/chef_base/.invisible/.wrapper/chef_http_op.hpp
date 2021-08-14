/**
 * @file     chef_http_op.hpp[_impl]
 * @deps     libcurl | chef_strings_op.hpp[_impl]
 * @platform linux/macos/xxx
 *
 * @author
 *   chef <191201771@qq.com>
 *     - initial release xxxx-xx-xx
 *
 * @brief  对libcurl做的wrapper，同步阻塞式完成http get/post
 *
 */

#ifndef _CHEF_BASE_HTTP_OP_H_
#define _CHEF_BASE_HTTP_OP_H_
#pragma once

#include "chef_strings_op.hpp"
#include <map>
#include <string>
#include <sstream>
#include <stdint.h>

namespace chef {

  class http_op {
    public:
      /**
       * @brief 内部调用curl_global_init，建议main函数初始化时调用有且仅有一次即可
       *
       * @NOTICE 关于libcurl的curl_global_init
       *         * 主要作用：初始化win32相关以及ssl部分
       *         * 根据curl文档上的描述，curl_global_init需要至少调用一次，且不能多线程调用。
       *         * 简单看了下libcurl的源码，curl_global_init确实存在一些无锁保护的全局状态。其实使用libcurl前不调用
       *         * curl_global_init在大部分时候也是可以的
       *         * 而且在curl_easy_init（即实际生成curl对象使用libcurl时）时会懒初始化调用curl_global_init，但很遗憾，也
       *           不是线程安全的。
       *         * 我本来想自己做个进程级once封装去调用。后来想想算了，因为应用程序可能在别的地方或别的库中也调用curl_global_init。
       *           所以封装一下,由用户自行保证~
       *         * 增加些许调用方的步骤，保证不出错~
       *
       *
       * @return 0 成功 -1 失败
       *
       */
      static int global_init_curl();

      static void global_cleanup_curl();

    public:
      struct response_cookie {
        std::string name_;
        std::string value_;
        std::string hostname_;
        bool        is_include_subdomains_;
        std::string path_;
        bool        is_secure_;
        int64_t     expiry_epoch_time_;

        std::string stringify() const {
          std::ostringstream ss;
          ss << "name:" << name_ << ",value:" << value_ << ",hostname:" << hostname_
             << ",is_include_subdomains:" << is_include_subdomains_ << ",path:" << path_ << ",is_secure:" << is_secure_
             << ",expiry_epoch_time:" << expiry_epoch_time_;
          return ss.str();
        }
      };

      struct response {
        int                                             code_;
        std::string                                     content_;
        std::map<std::string, std::string>              headers_;
        std::map<std::string, http_op::response_cookie> cookies_;
      };

      /**
       * @brief http get or post方法（见post_data参数），阻塞直至收到返回或超时~
       *
       * @param        url 'http://'前缀可以有也可以没有，query后缀可以有也可以没有
       * @param    headers 请求时的header 组，以kv结构传入，可为NULL，函数调用结束后，内部不持有该内存，可自由释放
       * @param    cookies 请求时的cookies组，以kv结构传入，可为NULL，函数调用结束后，内部不持有该内存，可自由释放
       * @param  post_data post数据，如果为NULL，则为get方法，函数调用结束后，内部不持有该内存，可自由释放
       * @param timeout_ms 超时时间，单位毫秒
       * @param       resp 返回结果，参见结构体http_op::response
       *
       * @return 0 成功 -1 失败
       *
       */
      static int get_or_post(const std::string &url,
                             const std::map<std::string, std::string> *headers,
                             const std::map<std::string, std::string> *cookies,
                             const char *post_data,
                             int timeout_ms,
                             response &resp /*out*/);

  };

} // namespace chef

#include "chef_http_op_impl.hpp"

#endif // _CHEF_BASE_HTTP_OP_H_
