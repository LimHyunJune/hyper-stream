#pragma once

class IVideoProcessor {
public:
    IVideoProcessor() = default;
    ~IVideoProcessor() = default;

    virtual bool initialize(shared_ptr<Channel<AVFramePtr>> src, shared_ptr<Channel<AVFramePtr>> dst) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};