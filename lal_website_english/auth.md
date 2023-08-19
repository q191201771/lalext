▌ Method 1. Fixed-value authentication parameters

First, modify the configuration file as follows:

```json
"conf_version": "v0.2.6", ...
...
"simple_auth": {
    "key": "q191201771",                    // Private key, used to calculate MD5 authentication parameters.
    "dangerous_lal_secret": "pengrl",       // Backdoor authentication parameter, all streams can be authenticated with this value.
    "pub_rtmp_enable": false,               // Whether or not RTMP push streams are enabled for authentication.
                                            // True to enable authentication, false to disable authentication.
    "sub_rtmp_enable": false,               // If RTMP pull is enabled or not.
    "sub_httpflv_enable": false,            // Whether to enable authentication for httpflv pulls.
    "sub_httpts_enable": false,             // If HTTP-TS pull is enabled or not.
    "pub_rtsp_enable": false,               // If RTSP push is enabled or not.
    "sub_rtsp_enable": false,               // If RTSP pull is enabled or not.
    "hls_m3u8_enable": true                 // If m3u8 pull is enabled or not.
}
...
```
The configuration for both mode 1 and mode 2 is managed under the configuration item `simple_auth`:

The fields beginning with `pub` and `sub` determine whether authentication is enabled for each protocol.
`dangerous_lal_secret` is the value of the authentication parameter.  
The `key` is used in Method 2, which we can ignore for now.  
For example, when authentication is not enabled, the URL to push the RTMP stream is `rtmp://127.0.0.1:8080/live/test110`.  
When it is turned on, the business side push stream URL is changed to
  rtmp://127.0.0.1:8080/live/test110?lal_secret=pengrl
where `lal_secret` is the authentication signature parameter, the name is fixed, the value is also fixed, corresponding to the value of `dangerous_lal_secret` in the configuration file.

With this authentication method, all streams use the same authentication parameter value.

▌ Method 2. MD5 Signed Authentication Parameters

First, the configuration file is still modified (see the description in Method 1 above):

The fields beginning with `pub`, `sub`, etc. determine whether or not authentication is enabled for each protocol.  
The `key` is used for the MD5 signature.  
For example, when authentication is not enabled, the URL to push the RTMP stream is `rtmp://127.0.0.1:8080/live/test110`.
After the function is enabled, the URL for the business side to push the stream is changed to
  rtmp://127.0.0.1:8080/live/test110?lal_secret=700997e1595a06c9ffa60ebef79105b0
where `lal_secret` is the authentication signature parameter, the name is fixed, and the value is calculated using the formula
  md5(simple_auth::key + stream name in the configuration file)

The above example corresponds to the calculation of
  md5("q191201771test110")

For Go, C++ code and how command-line and web tools calculate MD5 see: Generating MD5 using tools or code - Jen-Liang Notes

In this authentication mode, different stream names correspond to different authentication parameters.

Note that method 1 and method 2 can be used at the same time. `lal_secret` can be authenticated if its value meets any one of the calculation methods.

▌ Method 3. Interacting with Business Services to Customise Authentication Methods

The general logic is that lalserver uses HTTP callbacks to notify the business service backend when it receives a push or pull request.  
The business service determines whether the push or pull request is legitimate according to its own logic.  
If it is not legitimate, the request is denied and the connection terminated, using the HTTP API interface provided by lalserver.

Related Documentation:

[lalserver HTTP Notify event callbacks](HTTPNotify.md)
[lalserver HTTP API interface](HTTPAPI.md)

Third-party documentation:

- https://help.aliyun.com/document_detail/199349.html
- https://cloud.tencent.com/document/product/267/32735
- https://www.wangsu.com/document/99/171?rsr=ws

yoko, 20220115
