#pragma once

class VideoEncoder final : public IEncoder {
public:
    VideoEncoder() = default;
    ~VideoEncoder() = default;

    virtual bool initialize(shared_ptr<Channel<AVFramePtr>> src, shared_ptr<Channel<AVPacketPtr>> dst) override;
    virtual void run() override;
    virtual void stop() override;
};