#pragma once

class IMediaSink {
public:
    IMediaSink() = default;
    ~IMediaSink() = default;

    virtual bool initialize(shared_ptr<Channel<AVPacketPtr>> src) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};