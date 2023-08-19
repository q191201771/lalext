The lal project, in addition to `/app/lalserver` — the core services — also provides a number of additional small applications in the `/app/demo` directory, with the following (brief) functionality:

| demo              | Description                                                                          |
|-------------------|--------------------------------------------------------------------------------------|
| pushrtmp          | RTMP push stream client; stress test tool                                            |
| pullrtmp          | RTMP pull client; stress test tool                                                   |
| pullrtmp2pushrtmp | Pulls RTMP streams from remote servers, and use RTMP to push them out, support 1 to n push forward |
| pullrtmp2pushrtsp | Pulls RTMP streams from a remote server and pushes them out using RTSP.              |
| pullrtmp2hls      | Pulls RTMP stream from remote server and store it as local m3u8+ts file.             |
| pullhttpflv       | HTTP-FLV streaming client                                                            |
| pullrtsp          | RTSP streaming client                                                                |
| pullrtsp2pushrtsp | Pulls an RTSP stream from a remote server and pushes it out using RTSP transfer      |
| pullrtsp2pushrtmp | Pulls RTSP streams from remote server and pushes them out using RTMP transfer        |
|-------------------|--------------------------------------------------------------------------------------|
| benchrtmpconnect  | Stress test RTMP for concurrent connections.                                         |
| calcrtmpdelay     | Tests the delay between sending and receiving data from the RTMP server.             |
| analyseflv        | Pulls HTTP-FLV streams from a remote server and analyses them.                       |
| dispatch          | A simple demonstration of how to implement a simple dispatch service that allows multiple lalserver nodes to form a cluster. |
| flvfile2es        | Splits local FLV files into H264/AVC and AAC ES streams.                             |
| modflvfile        | Modifies some information of the FLV file (e.g. timestamps of some tags) and then saves the file. |

(for more specific functions, please refer to the header of each source file)