#pragma once

#include "Channel.h"
#include "AVMemory.h"
#include <string>

struct VideoEncoderParam {
    int width;
    int height;
    int bit_rate;
    AVBufferPtr hw_frames_ctx; 
    AVRational framerate;
    AVRational timebase;
    std::string preset;
    AVBufferPtr hw_device_ctx;
};

class IEncoder {
public:
    IEncoder() = default;
    ~IEncoder() = default;

    virtual bool initialize(VideoEncoderParam& param, shared_ptr<Channel<AVFramePtr>> src, shared_ptr<Channel<AVPacketPtr>> dst) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};