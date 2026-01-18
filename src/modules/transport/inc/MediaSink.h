#pragma once

#include "IMediaSink.h"

class MediaSink final : public IMediaSink{

private:
    AVFormatContext* output_ctx = nullptr;
    bool is_stop = false;
    AVStream* stream = nullptr;
public:
    MediaSink() = default;
    ~MediaSink();

    virtual bool initialize(MediaSinkParam& param, shared_ptr<Channel<AVPacketPtr>> src) override;
    virtual void run() override;
    virtual void stop() override;
};