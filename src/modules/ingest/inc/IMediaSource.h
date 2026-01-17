#pragma once

class IMediaSource {
public:
    IMediaSource() = default;
    ~IMediaSource() = default;

    virtual bool initialize(shared_ptr<Channel<AVPacketPtr>> dst) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
}; 