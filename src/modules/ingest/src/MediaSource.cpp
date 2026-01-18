#include "MediaSource.h"
#include "Logger.h"

MediaSource::~MediaSource()
{
    avformat_free_context(input_ctx);
}

bool MediaSource::initialize(MediaSourceParam& param, shared_ptr<Channel<AVPacketPtr>> dst)
{
    this->param = param;
    this->dst = dst;
    
    int ret = avformat_open_input(&input_ctx, param.url.c_str(), nullptr, nullptr);
    if(ret < 0)
    {
        BOOST_LOG(error) << "[INGEST{ avformat_open_input failed";
        return false;
    }
    ret = avformat_find_stream_info(input_ctx, nullptr);
    if(ret < 0)
    {
        BOOST_LOG(error) << "[INGEST] avformat_find_stream_info failed";
        return false;
    }
    return true;
}

vector<AVStream*> MediaSource::get_video_streams()
{
    vector<AVStream*> video_streams;
    for (unsigned int i = 0; i < input_ctx->nb_streams; i++) {
        if (input_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_streams.push_back(input_ctx->streams[i]);
        }
    }
    return video_streams;
}


void MediaSource::run()
{
    BOOST_LOG(debug) << "[INGEST] running";
    while(!is_stop)
    {
        AVPacketPtr pkt_ptr(av_packet_alloc());
        auto pkt = pkt_ptr.get();

        int ret = av_read_frame(input_ctx, pkt);
        if(ret >= 0)
        {
            if(input_ctx->streams[pkt->stream_index]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO)
                continue;
            BOOST_LOG(debug) << "[INGEST] pts : " << pkt->pts << " dts : " << pkt->dts << " size : " << pkt->size;
            dst->push(std::move(pkt_ptr));
        }

        else
        {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
            BOOST_LOG(debug) << "[INGEST] read failed: " << errbuf << " (" << ret << ")";
            break;
        }
    }
    BOOST_LOG(debug) << "[INGEST] stopped";
    dst.get()->stop();
}

void MediaSource::stop()
{
    is_stop = true;
}

