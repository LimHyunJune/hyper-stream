#pragma once

#include "IDecoder.h"
#include "Channel.h"
#include "AVMemory.h"
#include <map>

class VideoDecoder final : public IDecoder {
public:
    VideoDecoder() = default;
    ~VideoDecoder() = default;

    virtual bool initialize(DecoderParam& param, shared_ptr<Channel<AVPacketPtr>> src, shared_ptr<Channel<AVFramePtr>> dst) override;
    virtual void run() override;
    virtual void stop() override;

private:
    shared_ptr<Channel<AVPacketPtr>> src;
    shared_ptr<Channel<AVFramePtr>> dst;
    std::map<int, AVCodecContext*> codec_contexts;
    bool is_stop = false;
};