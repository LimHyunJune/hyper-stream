#include "Controller.h"
#include <vector>

Controller::Controller()
{
    initialize();
}

bool Controller::init_hw_device()
{
    hw_device_ctx = av_hwdevice_context_alloc(AV_HWDEVICE_TYPE_CUDA);
    if(!hw_device_ctx)
    {
        BOOST_LOG(error) << "[CONTROLLER] av_hwdevice_context_alloc failed";
        return false;
    }

    hw_frames_ctx = av_hwframe_context_alloc(hw_device_ctx);
    if(!hw_frames_ctx)
    {
        BOOST_LOG(error) << "[CONTROLLER] av_hwframe_context_alloc failed";
        return false;
    }

    AVHWFrameContext* hw_frame_ctx_data = hw_frames_ctx.get()->data;
    hw_frame_ctx_data->format = AV_PIX_FMT_CUDA;
    hw_frame_ctx_data->width = 3840;
    hw_frame_ctx_data->height = 2160;
    hw_frame_ctx_data->initial_pool_size = 50;
    
    if(av_hwframe_context_init(hw_frames_ctx) < 0)
    {
        BOOST_LOG(error) << "[CONTROLLER] av_hwframe_context_init failed";
        return false;
    }
    return true;
}

void Controller::initialize()
{
    if(!init_hw_device())
    {
        BOOST_LOG(error) << "[CONTROLLER] init_hw_device failed";
        return;
    }

    ingest_channel = make_shared<Channel<AVPacketPtr>>();
    decode_channel = make_shared<Channel<AVFramePtr>>();
    filter_channel = make_shared<Channel<AVFramePtr>>();
    encode_channel = make_shared<Channel<AVPacketPtr>>();


    source = make_unique<MediaSource>();
    sink = make_unique<MediaSink>();
    decoder = make_unique<VideoDecoder>();
    encoder = make_unique<VideoEncoder>();
    processor = make_unique<VideoProcessor>();

    processor.get()->initialize(param.processor_param, decode_channel, filter_channel);
    encoder.get()->initialize(param.encoder_param, encode_channel, sink);
    sink.get()->initialize(param.sink_param, encode_channel);
    source.get()->initialize(param.source_param, ingest_channel);
    decoder.get()->initialize(param.decoder_param, ingest_channel, decode_channel);
}

void Controller::run()
{
    vector<thread> threads;
    threads.emplace_back(&MediaSource::run, source.get());
    threads.emplace_back(&VideoDecoder::run, decoder.get());
    threads.emplace_back(&VideoProcessor::run, processor.get());
    threads.emplace_back(&VideoEncoder::run, encoder.get());
    threads.emplace_back(&MediaSink::run, sink.get());

    for(auto& t : threads)
        t.join();
}

void Controller::stop()
{
    source.get()->stop();
    decoder.get()->stop();
    processor.get()->stop();
    encoder.get()->stop();
    sink->stop();
}