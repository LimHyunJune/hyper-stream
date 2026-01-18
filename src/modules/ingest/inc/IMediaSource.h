#pragma once

extern "C"{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}

#include "Channel.h"
#include "AVMemory.h"
#include <memory>
#include <string>

using namespace std;

struct MediaSourceParam {
    string url;
};    

class IMediaSource {

protected:
    shared_ptr<Channel<AVPacketPtr>> dst;
    MediaSourceParam param;

public:
    IMediaSource() = default;
    ~IMediaSource() = default;

    virtual bool initialize(MediaSourceParam& param, shared_ptr<Channel<AVPacketPtr>> dst) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
}; 