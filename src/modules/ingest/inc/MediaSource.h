#pragma once

class MediaSource final : public IMediaSource {
public:
    MediaSource() = default;
    ~MediaSource() = default;

    virtual bool initialize(shared_ptr<Channel<AVPacketPtr>> dst) override;
    virtual void run() override;
    virtual void stop() override;
};