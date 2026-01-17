#pragma once

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}
#include <memory>

using namespace std;

struct AVMemoryDeleter {
    void operator()(AVPacket* ptr) const {
        av_packet_free(&ptr);
    }
    void operator()(AVFrame* ptr) const {
        av_frame_free(&ptr);
    }
    void operator()(AVBufferRef* ptr) const {
        av_buffer_unref(&ptr);
    }
};

using AVPacketPtr = std::unique_ptr<AVPacket, AVMemoryDeleter>;
using AVFramePtr = std::unique_ptr<AVFrame, AVMemoryDeleter>;
using AVBufferPtr = std::shared_ptr<AVBufferRef>;
