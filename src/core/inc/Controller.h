#pragma once

#include "IMediaSource.h"
#include "IMediaSink.h"
#include "IDecoder.h"
#include "IEncoder.h"
#include "IVideoProcessor.h"

#include "AVMemory.h"
#include "Channel.h"
#include "Logger.h"

struct ControllerParam {
    MediaSourceParam source_param;
    MediaSinkParam sink_param;
    // VideoDecoderParam decoder_param;
    // VideoEncoderParam encoder_param;
    // VideoProcessorParam processor_param;
};

class Controller
{
    private:
        ControllerParam param;

        BufferPtr hw_device_ctx;
        BufferPtr hw_frames_ctx;

        shared_ptr<Channel<AVPacketPtr>> ingest_channel;
        shared_ptr<Channel<AVFramePtr>> decode_channel;
        shared_ptr<Channel<AVFramePtr>> filter_channel;
        shared_ptr<Channel<AVPacketPtr>> encode_channel;
        
        unique_ptr<MediaSource> source;
        unique_ptr<MediaSink> sink;
        unique_ptr<IVideoDecoder> decoder;
        unique_ptr<IVideoEncoder> encoder;
        unique_ptr<IVideoProcessor> processor;

        Controller();
        ~Controller() = default;

        bool init_hw_device();
        void initialize();

    public:
        static Controller& get_instance()
        {
            static Controller instance;
            return instance;
        }

        BufferPtr get_hw_device_ctx() { return hw_device_ctx; }
        BufferPtr get_hw_frames_ctx() { return hw_frames_ctx; }

        bool initialize(ControllerParam& param);
        void run();
        void stop();
};