#pragma once

class MultiviewCompositor final : public IVideoProcessor {
public:
    MultiviewCompositor() = default;
    ~MultiviewCompositor() = default;

    virtual bool initialize(shared_ptr<Channel<AVFramePtr>> src, shared_ptr<Channel<AVFramePtr>> dst) override;
    virtual void run() override;
    virtual void stop() override;
};