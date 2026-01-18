#include "Controller.h"
#include <vector>
#include <thread>
#include "MediaSource.h"
#include "MediaSink.h"
#include "VideoDecoder.h"
#include "VideoEncoder.h"
#include "MultiviewCompositor.h"

Controller::Controller()
{
    initialize();
}

Controller::~Controller() = default;

bool Controller::init_hw_device()
{
    AVBufferRef* device_ctx = nullptr;
    int ret = av_hwdevice_ctx_create(&device_ctx, AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0);
    if(ret < 0)
    {
        BOOST_LOG(error) << "[CONTROLLER] av_hwdevice_ctx_create failed";
        return false;
    }
    hw_device_ctx = AVBufferPtr(device_ctx, AVMemoryDeleter());

    AVBufferRef* frames_ctx = av_hwframe_ctx_alloc(device_ctx);
    if(!frames_ctx)
    {
        BOOST_LOG(error) << "[CONTROLLER] av_hwframe_ctx_alloc failed";
        return false;
    }
    hw_frames_ctx = AVBufferPtr(frames_ctx, AVMemoryDeleter());

    AVHWFramesContext* hw_frame_ctx_data = (AVHWFramesContext*)frames_ctx->data;
    hw_frame_ctx_data->format = AV_PIX_FMT_CUDA;
    hw_frame_ctx_data->sw_format = AV_PIX_FMT_NV12; 
    hw_frame_ctx_data->width = 3840;
    hw_frame_ctx_data->height = 2160;
    hw_frame_ctx_data->initial_pool_size = 50;
    
    if(av_hwframe_ctx_init(frames_ctx) < 0)
    {
        BOOST_LOG(error) << "[CONTROLLER] av_hwframe_ctx_init failed";
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
    processor = make_unique<MultiviewCompositor>();

    processor.get()->initialize(param.processor_param, decode_channel, filter_channel);
    encoder.get()->initialize(param.encoder_param, encode_channel, sink);
    sink.get()->initialize(param.sink_param, encode_channel);
    source.get()->initialize(param.source_param, ingest_channel);
    decoder.get()->initialize(param.decoder_param, ingest_channel, decode_channel);
}

void Controller::run()
{
    vector<thread> threads;
    threads.emplace_back(&IMediaSource::run, source.get());
    threads.emplace_back(&IDecoder::run, decoder.get());
    threads.emplace_back(&IVideoProcessor::run, processor.get());
    threads.emplace_back(&IEncoder::run, encoder.get());
    threads.emplace_back(&IMediaSink::run, sink.get());

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