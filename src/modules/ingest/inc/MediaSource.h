#pragma once

#include "IMediaSource.h"

class MediaSource final : public IMediaSource {
private:
    AVFormatContext* input_ctx = nullptr;
    bool is_stop = false;
public:
    MediaSource() = default;
    ~MediaSource();

    virtual vector<AVStream*> get_video_streams() override;
    
    virtual bool initialize(MediaSourceParam& param, shared_ptr<Channel<AVPacketPtr>> dst) override;
    virtual void run() override;
    virtual void stop() override;
};