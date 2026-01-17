#pragma once

class VideoDecoder final : public IDecoder {
public:
    VideoDecoder() = default;
    ~VideoDecoder() = default;

    virtual bool initialize(shared_ptr<Channel<AVPacketPtr>> src, shared_ptr<Channel<AVFramePtr>> dst) override;
    virtual void run() override;
    virtual void stop() override;
};