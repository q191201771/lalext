#include <stdio.h>

extern "C" {
    #include "libavformat/avformat.h"
}

int main() {
    AVFormatContext *fmtCtx=NULL;
    int ret = avformat_open_input(&fmtCtx, "rtmp://127.0.0.1/live/test110", NULL, NULL);
    printf("%d\n", ret);

    return 0;
}
