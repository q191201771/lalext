// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _LALCC_ENCODE_HPP_
#define _LALCC_ENCODE_HPP_

#include "lalcc_forward_declaration.hpp"
#include "lalcc_av_packet_t.hpp"

namespace lalcc {

  EncodePtr NewEncode();

  class Encode {
  public:
    int Open(AVStream *stream);
    int Push(AvPacketTPtr pkt);

  private:
    AVCodecContext *codecCtx_ = nullptr;
    SwrContext *swrCtx_ = nullptr;
    FILE *fp_ = nullptr;
  };

}

// --------------------------------------------------------------------------------------------------------------------

namespace lalcc {

  inline int Encode::Open(AVStream *stream) {
    return 0;
  }

  inline int Encode::Push(AvPacketTPtr pkt) {
    return 0;
  }

  inline EncodePtr NewEncode() {
    return chef::make_shared<Encode>();
  }

}

#endif