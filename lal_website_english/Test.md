# Performance Test
**Test Scenario 1:** Continuously push *n* RTMP streams to lalserver (no pull streams)

| Number of streams pushed  | CPU usage | Memory usage (RES) |
|---------------------------|----------:|-------------------:|
| 1000 (from a single core) |       16% |              104MB |

**Test Scenario 2:** Continuously push 1 RTMP stream to lalserver and pull *n* streams from lalserver using the RTMP protocol.

| Number of streams pulled  | CPU usage | Memory usage (RES) |
|---------------------------|----------:|-------------------:|
| 1000 (from a single core) |       30% |              120MB |

**Test Scenario 3:** Continuously push *n*-channel RTMP streams to the lalserver, and pull *n*-channel streams from the lalserver using the RTMP protocol (the push and pull streams are in a 1-to-1 relationship).

| Number of Push Streams | Number of Pull Streams | CPU usage | Memory usage (RES) |
|------------------------|------------------------|----------:|-------------------:|
| 1000                   | 1000                   |      125% |              464MB |

Test machine: 32 cores and 16G (lalserver server and pressure test tool are running on this machine at the same time)  
Pressure test tools: `/app/demo/pushrtmp` and `/app/demo/pullrtmp` in lal.  
Pushstream bitrate: roughly 200kbps  
lalserver version: based on git commit: xxx

Since the test machine is a shared machine with many other services running on it, the data listed here is just a rough idea, and more performance analysis and optimisation is still to be done. If you're interested in evaluating performanceg, feel free to test it and send me your results.

Performance and readability are sometimes contradictory, and there are trade-offs. I'll keep thinking and try to balance the two.

## Third-party clients tested

### RTMP pushstream side:
- OBS 21.0.3 (macOS)
- OBS 24.0.3 (Win10 64-bit)
- ffmpeg 3.4.2 (macOS)
- srs-bench (a pressure testing tool for the macOS srs project)
- pushrtmp (RTMP push streaming client in macOS lal demo)

### RTMP / HTTP-FLV pull streaming client:
- VLC 3.0.8 (macOS 10.15.1)
- VLC 3.0.8 (Win10)
- MPV 0.29.1 (macOS)
- ffmpeg 3.4.2 (macOS)
- srs-bench (a pressure-testing tool that comes with the macos srs project)
