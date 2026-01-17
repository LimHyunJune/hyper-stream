#pragma once

class IDecoder {
public:
    IDecoder() = default;
    ~IDecoder() = default;

    virtual bool initialize(shared_ptr<Channel<AVPacketPtr>> src, shared_ptr<Channel<AVFramePtr>> dst) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};
