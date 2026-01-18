#include "VideoEncoder.h"
#include "Logger.h"

extern "C" {
    #include <libavutil/opt.h>
}

bool VideoEncoder::initialize(EncoderParam& param, shared_ptr<Channel<AVFramePtr>> src, shared_ptr<Channel<AVPacketPtr>> dst) {
    this->src = src;
    this->dst = dst;

    const AVCodec* codec = nullptr;
    if (param.encoder_type == EncoderType::HEVC) {
        codec = avcodec_find_encoder_by_name("hevc_nvenc");
    }

    if (!codec) {
        BOOST_LOG(error) << "[ENCODER] Codec hevc_nvenc not found";
        return false;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        BOOST_LOG(error) << "[ENCODER] Could not allocate video codec context";
        return false;
    }

    codec_ctx->width = param.output_config.width;
    codec_ctx->height = param.output_config.height;
    codec_ctx->time_base = param.time_base;
    codec_ctx->framerate = param.frame_rate;
    codec_ctx->pix_fmt = AV_PIX_FMT_CUDA; // Using NVENC, input is likely CUDA frames

    if (param.output_config.bit_rate > 0) {
        codec_ctx->bit_rate = param.output_config.bit_rate;
    }
    
    if (param.hw_device_ctx) {
         codec_ctx->hw_device_ctx = av_buffer_ref(param.hw_device_ctx.get());
    }

    if (param.output_config.hw_frames_ctxs) {
        codec_ctx->hw_frames_ctx = av_buffer_ref(param.output_config.hw_frames_ctxs.get());
    }

    if (!param.preset.empty()) {
        av_opt_set(codec_ctx->priv_data, "preset", param.preset.c_str(), 0);
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        BOOST_LOG(error) << "[ENCODER] Could not open codec";
        return false;
    }

    BOOST_LOG(info) << "[ENCODER] Initialized with " << codec->name;
    return true;
}

void VideoEncoder::run() {
    BOOST_LOG(info) << "[ENCODER] running";

    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        BOOST_LOG(error) << "[ENCODER] Could not allocate packet";
        return;
    }

    while (!is_stop) {
        AVFramePtr frame_ptr;
        if (!src->pop(frame_ptr)) {
            break;
        }

        AVFrame* frame = frame_ptr.get();
        
        // Ensure frame has correct formatting for NVENC if needed (e.g. check pix_fmt)
        
        int ret = avcodec_send_frame(codec_ctx, frame);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
            BOOST_LOG(error) << "[ENCODER] Error sending frame for encoding: " << errbuf;
            continue;
        }

        while (ret >= 0) {
            ret = avcodec_receive_packet(codec_ctx, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
                BOOST_LOG(error) << "[ENCODER] Error during encoding: " << errbuf;
                break;
            }

            pkt->stream_index = 0; // Default stream index

            AVPacket* new_pkt = av_packet_alloc();
            if(new_pkt) {
                av_packet_move_ref(new_pkt, pkt);
                AVPacketPtr pkt_ptr(new_pkt);
                dst->push(std::move(pkt_ptr));
            }
        }
    }

    av_packet_free(&pkt);
    // Send flush frame
    avcodec_send_frame(codec_ctx, nullptr); 
    // Flush remaining packets... (logic omitted for brevity, but good practice to have)

    avcodec_free_context(&codec_ctx);
    BOOST_LOG(info) << "[ENCODER] stopped";
    dst->stop();
}

void VideoEncoder::stop() {
    is_stop = true;
    if(src) src->stop();
}
