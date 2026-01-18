#pragma once

#include "Channel.h"
#include "AVMemory.h"
#include <string>

enum class EncoderType {
    HEVC
};

struct EncoderOut {
    int width;
    int height;
    int bit_rate;
    AVBufferPtr hw_frames_ctxs; 
};

struct EncoderParam {
    EncoderOut output_config;
    AVRational frame_rate;
    AVRational time_base;
    std::string preset;
    AVBufferPtr hw_device_ctx;
    EncoderType encoder_type;
};

class IEncoder {
public:
    IEncoder() = default;
    ~IEncoder() = default;

    virtual bool initialize(EncoderParam& param, shared_ptr<Channel<AVFramePtr>> src, shared_ptr<Channel<AVPacketPtr>> dst) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};