#pragma once

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}

#include "AVMemory.h"
#include "Channel.h"
#include <string>

using namespace std;


struct MediaSinkParam {

    enum class Protocol {
        SRT,
        FILE
    };
    
    string url;
    Protocol protocol;
    AVCodecContext* enc_ctx;
};

class IMediaSink {
protected:
    MediaSinkParam param;
    shared_ptr<Channel<AVPacketPtr>> src;

public:
    IMediaSink() = default;
    ~IMediaSink() = default;

    virtual bool initialize(MediaSinkParam& param, shared_ptr<Channel<AVPacketPtr>> src) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};