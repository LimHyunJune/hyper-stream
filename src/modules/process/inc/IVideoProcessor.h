#pragma once

extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavfilter/avfilter.h>
}

#include "AVMemory.h"
#include "Channel.h"

struct VideoProcessorParam {
    AVRational framerate;
    AVRational timebase;
    int width;
    int height;
    AVBufferPtr hw_frames_ctx;
};

class IVideoProcessor {
public:
    IVideoProcessor() = default;
    ~IVideoProcessor() = default;

    virtual bool initialize(VideoProcessorParam& param, shared_ptr<Channel<AVFramePtr>> src, shared_ptr<Channel<AVFramePtr>> dst) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};