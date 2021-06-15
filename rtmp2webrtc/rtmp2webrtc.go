// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"net/http"

	"github.com/q191201771/naza/pkg/nazahttp"
	"github.com/q191201771/naza/pkg/nazalog"
)

type TaskParam struct {
	RtmpUrl            string `json:"rtmp_url"`
	SessionDescription string `json:"session_description"`
}

type TaskResponse struct {
	SessionDescription string `json:"session_description"`
}

func main() {
	_ = nazalog.Init(func(option *nazalog.Option) {
		option.Level = nazalog.LevelTrace
	})

	port := parseFlag()
	addr := fmt.Sprintf(":%d", port)

	var m http.ServeMux

	var pageFn = func(writer http.ResponseWriter, request *http.Request) {
		buf, err := ioutil.ReadFile("rtmp2webrtc.html")
		if err != nil {
			buf, err = ioutil.ReadFile("rtmp2webrtc/rtmp2webrtc.html")
		}
		if err != nil {
			writer.WriteHeader(404)
			return
		}
		_, _ = writer.Write(buf)
	}

	m.HandleFunc("/", pageFn)
	m.HandleFunc("/rtmp2webrtc.html", pageFn)

	m.HandleFunc("/task", func(writer http.ResponseWriter, request *http.Request) {
		var param TaskParam
		err := nazahttp.UnmarshalRequestJsonBody(request, &param, "rtmp_url", "session_description")
		if err != nil {
			nazalog.Error(err)
			return
		}

		sessDescChan := make(chan string, 1)
		errChan := make(chan error, 1)
		go func() {
			err = StartTunnelTask(param.RtmpUrl, param.SessionDescription, func(sessDesc string) {
				sessDescChan <- sessDesc
			})
			errChan <- err
		}()
		select {
		case sessDesc := <-sessDescChan:
			b, err := json.Marshal(TaskResponse{
				SessionDescription: sessDesc,
			})
			nazalog.Assert(nil, err)
			_, err = writer.Write(b)
			nazalog.Assert(nil, err)
		case err := <-errChan:
			nazalog.Assert(nil, err)
		}
	})
	nazalog.Infof("> start listen, open http://127.0.0.1:%d/rtmp2webrtc.html", port)
	err := http.ListenAndServe(addr, &m)
	nazalog.Assert(nil, err)
}

func parseFlag() int {
	port := flag.Int("p", 8827, "specify listen port")
	flag.Parse()
	return *port
}
