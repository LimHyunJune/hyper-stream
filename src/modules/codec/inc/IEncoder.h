#pragma once

class IEncoder {
public:
    IEncoder() = default;
    ~IEncoder() = default;

    virtual bool initialize(shared_ptr<Channel<AVFramePtr>> src, shared_ptr<Channel<AVPacketPtr>> dst) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};