#include "IEncoder.h"
#include "Channel.h"
#include "AVMemory.h"

class VideoEncoder final : public IEncoder {
public:
    VideoEncoder() = default;
    ~VideoEncoder() = default;

    virtual bool initialize(VideoEncoderParam& param, shared_ptr<Channel<AVFramePtr>> src, shared_ptr<Channel<AVPacketPtr>> dst) override;
    virtual void run() override;
    virtual void stop() override;

private:
    shared_ptr<Channel<AVFramePtr>> src;
    shared_ptr<Channel<AVPacketPtr>> dst;
    AVCodecContext* codec_ctx = nullptr;
    bool is_stop = false;
};