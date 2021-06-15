// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lalext
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package main

import (
	"encoding/base64"
	"encoding/json"

	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/naza/pkg/bele"
)

// TODO(chef) move to lal
func packStapA(nals [][]byte) []byte {
	length := 1
	for _, nal := range nals {
		length += 2 + len(nal)
	}
	ret := make([]byte, length)
	ret[0] = uint8(rtprtcp.NaluTypeAvcStapa) | (nals[0][0] & 0xE0)
	index := 1
	for _, nal := range nals {
		bele.BePutUint16(ret[index:], uint16(len(nal)))
		copy(ret[index+2:], nal)
		index += 2 + len(nal)
	}
	return ret
}

func decodeBase64Json(in string, out interface{}) error {
	raw, err := base64.StdEncoding.DecodeString(in)
	if err != nil {
		return err
	}
	return json.Unmarshal(raw, out)
}

func encodeBase64Json(in interface{}) (string, error) {
	raw, err := json.Marshal(in)
	if err != nil {
		return "", err
	}
	return base64.StdEncoding.EncodeToString(raw), nil
}
