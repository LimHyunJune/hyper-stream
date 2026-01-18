#pragma once

#include "Channel.h"
#include "AVMemory.h"
#include <vector>

struct VideoDecoderParam {
    std::vector<AVStream*> video_streams;
    AVBufferPtr hw_device_ctx;
    AVBufferPtr hw_frames_ctx;
};

class IDecoder {
public:
    IDecoder() = default;
    ~IDecoder() = default;

    virtual bool initialize(VideoDecoderParam& param, shared_ptr<Channel<AVPacketPtr>> src, shared_ptr<Channel<AVFramePtr>> dst) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};
