#include "VideoDecoder.h"
#include "Logger.h"
#include "Metadata.h"

bool VideoDecoder::initialize(VideoDecoderParam& param, shared_ptr<Channel<AVPacketPtr>> src, shared_ptr<Channel<AVFramePtr>> dst) {
    this->src = src;
    this->dst = dst;

    if (param.video_streams.empty()) {
        BOOST_LOG(error) << "[DECODER] No video streams provided";
        return false;
    }

    for (auto stream : param.video_streams) {
        const AVCodec* codec = nullptr; 

        if (stream->codecpar->codec_id == AV_CODEC_ID_HEVC) {
            codec = avcodec_find_decoder_by_name("hevc_nvdec");
        } else {
            codec = avcodec_find_decoder(stream->codecpar->codec_id);
        }

        if (!codec) {
            BOOST_LOG(error) << "[DECODER] Codec not found for stream " << stream->index;
            continue;
        }

        AVCodecContext* ctx = avcodec_alloc_context3(codec);
        if (!ctx) {
            BOOST_LOG(error) << "[DECODER] Could not allocate video codec context for stream " << stream->index;
            continue;
        }

        if (avcodec_parameters_to_context(ctx, stream->codecpar) < 0) {
            BOOST_LOG(error) << "[DECODER] Could not copy codec parameters for stream " << stream->index;
            avcodec_free_context(&ctx);
            continue;
        }

        if (param.hw_device_ctx) {
            ctx->hw_device_ctx = av_buffer_ref(param.hw_device_ctx.get());
        }

        if (param.hw_frames_ctx) {
            ctx->hw_frames_ctx = av_buffer_ref(param.hw_frames_ctx.get());
        }
        ctx->thread_count = 1;

        if (avcodec_open2(ctx, codec, nullptr) < 0) {
            BOOST_LOG(error) << "[DECODER] Could not open codec for stream " << stream->index;
            avcodec_free_context(&ctx);
            continue;
        }

        codec_contexts[stream->index] = ctx;
        BOOST_LOG(info) << "[DECODER] Initialized stream " << stream->index << " with " << codec->name;
    }

    if (codec_contexts.empty()) {
        BOOST_LOG(error) << "[DECODER] No streams initialized successfully";
        return false;
    }

    return true;
}

void VideoDecoder::run() {
    BOOST_LOG(info) << "[DECODER] running";

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        BOOST_LOG(error) << "[DECODER] Could not allocate frame";
        return;
    }

    while (!is_stop) {
        AVPacketPtr pkt_ptr;
        if (!src->pop(pkt_ptr)) {
            break;
        }
        
        AVPacket* pkt = pkt_ptr.get();

        int idx = pkt->stream_index;

        if (codec_contexts.find(idx) == codec_contexts.end()) {
             // Packet belongs to a stream we aren't decoding (e.g. audio) or failed init
             continue; 
        }

        int ret = avcodec_send_packet(codec_contexts[idx], pkt);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
            BOOST_LOG(error) << "[DECODER] Error sending packet for decoding: " << errbuf;
            continue;
        }

        while (true) {
            AVFramePtr frame_ptr(av_frame_alloc());
			auto frame = frame_ptr.get();

            ret = avcodec_receive_frame(codec_contexts[idx], frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
                BOOST_LOG(error) << "[DECODER] Error during decoding: " << errbuf;
                break;
            }

            frame->opaque = static_cast<void*>(new Metadata{idx});
            dst->push(std::move(frame_ptr));

        }
    }

    av_frame_free(&frame);
    for (auto& pair : codec_contexts) {
        avcodec_free_context(&pair.second);
    }
    codec_contexts.clear();

    BOOST_LOG(info) << "[DECODER] stopped";
    dst->stop();
}

void VideoDecoder::stop() {
    is_stop = true;
    if(src) src->stop(); 
}
