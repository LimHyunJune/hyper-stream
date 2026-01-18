#include "MediaSink.h"
#include "Logger.h"

MediaSink::~MediaSink()
{
    avformat_free_context(output_ctx);
}

bool MediaSink::initialize(MediaSinkParam& param, shared_ptr<Channel<AVPacketPtr>> src)
{
    this->param = param;
    this->src = src;

    avformat_alloc_output_context2(&output_ctx, nullptr, "mpegts", param.url.c_str());
    if(!output_ctx)
    {
        BOOST_LOG(error) << "[TRANSPORT] avformat_alloc_output_context2 failed";
        return false;
    }

    stream = avformat_new_stream(output_ctx, nullptr);
    if(!stream)
    {
        BOOST_LOG(error) << "[TRANSPORT] avformat_new_stream failed";
        return false;
    }  

    avcodec_parameters_from_context(stream->codecpar, param.enc_ctx);
    stream->time_base = param.enc_ctx->time_base;
    stream->avg_frame_rate = param.enc_ctx->framerate;

    AVDictionary* io_options = nullptr;
    av_dict_set(&io_options, "reuse", "1", 0);
    av_dict_set(&io_options, "latency", "2000", 0);
    av_dict_set(&io_options, "pkt_size", "1316", 0);
    av_dict_set(&io_options, "rcvbuf", "1073741824", 0);

    AVIOContext* io_ctx = nullptr;
    avio_open2(&io_ctx, param.url.c_str(), AVIO_FLAG_WRITE, NULL, &io_options);
    if(!io_ctx)
    {
        BOOST_LOG(error) << "[TRANSPORT] avio_open failed";
        return false;
    }
    output_ctx->pb = io_ctx;
    av_dict_free(&io_options);

    AVDictionary* options = nullptr;
    av_dict_set(&options, "mpegts_flags", "+resend_headers+initial_discontinuity", 0);
    av_dict_set(&options, "pat_period", "0.5", 0);
    av_dict_set(&options, "pmt_period", "0.5", 0);
    av_dict_set(&options, "pcr_period", "20", 0);
    av_dict_set(&options, "flush_packets", "1", 0);
    av_dict_set(&options, "mpegts_copyts", "1", 0);

    int ret;
    if((ret = avformat_write_header(output_ctx, &options)) < 0)
    {
        BOOST_LOG(error) << "[TRANSPORT] avformat_write_header failed";
        return false;
    }
    av_dict_free(&options);
    return true;
}


void MediaSink::run()
{
    BOOST_LOG(debug) << "[TRANSPORT] running";
    AVPacketPtr pkt_ptr;
    while(!is_stop && src.get()->pop(pkt_ptr))
    {
        auto pkt = pkt_ptr.get();

        pkt->stream_index = 0;

        pkt->pts = av_rescale_q(pkt->pts, param.enc_ctx->time_base, stream->time_base);
        pkt->dts = av_rescale_q(pkt->dts, param.enc_ctx->time_base, stream->time_base);
        pkt->duration = av_rescale_q(pkt->duration, param.enc_ctx->time_base, stream->time_base);

        av_interleaved_write_frame(output_ctx, pkt);
    }
    av_write_trailer(output_ctx);
    BOOST_LOG(debug) << "[TRANSPORT] stopped";
}

void MediaSink::stop()
{
    is_stop = true;
}