// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
#ifndef _LALCC_AV_PACKET_T_HPP_
#define _LALCC_AV_PACKET_T_HPP_

#include "lalcc_forward_declaration.hpp"

namespace lalcc {
  AvPacketTPtr NewAvPacketT();
  AvPacketTPtr NewAvPacketT(AVPacket *p);

  class AvPacketT {
    public:
      AvPacketT() { p_ = av_packet_alloc(); }
      AvPacketT(AVPacket *p) : p_(p) {}
      ~AvPacketT() { av_packet_free(&p_); }

      AvPacketTPtr Clone() {
        return NewAvPacketT(av_packet_clone(p_));
      }

      AVPacket *Core() { return p_; }

    private:
      AVPacket *p_;
  };

}

// --------------------------------------------------------------------------------------------------------------------

namespace lalcc {
  AvPacketTPtr NewAvPacketT() {
    return chef::make_shared<AvPacketT>();
  }

  AvPacketTPtr NewAvPacketT(AVPacket *p) {
    return chef::make_shared<AvPacketT>(p);
  }
}

#endif
