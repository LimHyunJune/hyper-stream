#include "MultiviewCompositor.h"

MultiviewCompositor::~MultiviewCompositor()
{
    avfilter_graph_free(&filter_graph);
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
}

void MultiviewCompositor::make_multiview_compositor()
{
    filter_descr += "[v0]scale_cuda=2560:1440,hwdownload,format=nv12,pad=3840:2160:100:360:#001219[main];";
    filter_descr += "[v1]scale_cuda=960:540,hwdownload,format=nv12,fps=60[sub1];";
    filter_descr += "[v2]scale_cuda=960:540,hwdownload,format=nv12,fps=60[sub2];";
    filter_descr += "[v3]scale_cuda=960:540,hwdownload,format=nv12,fps=60[sub3];";
    filter_descr += "[main][sub1]overlay_cuda=2760:120:repeatlast=1[o2];";
    filter_descr += "[o2][sub2]overlay_cuda=2760:720:repeatlast=1[o3];";
    filter_descr += "[o3][sub3]overlay_cuda=2760:1320:repeatlast=1[out]";
}

void MultiviewCompositor::create_filter_graph(int idx)
{
    char name[8];
    snprintf(name, sizeof(name), "v%d", idx);

    AVPixelFormat format = AV_PIX_FMT_CUDA;
    char args[512];
    snprintf(args, sizeof(args),
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:frame_rate=%d/%d:pixel_aspect=1",
        param.width, param.height, format, param.timebase.num, param.timebase.den, param.framerate.num, param.framerate.den
    );

    avfilter_graph_create_filter(&buffer_ctxs[idx], avfilter_get_by_name("buffer"), name, args, nullptr, filter_graph);

    AVBufferSrcParameters param;
    memset(&param, 0, sizeof(param));
    param.hw_frames_ctx = param.hw_frames_ctx.get();
    param.format = format;
    int ret = av_buffersrc_parameters_set(buffer_ctxs[idx], &param);
    if(ret < 0)
    {
        BOOST_LOG(error) << "[MULTIVIEW_COMPOSITOR] av_buffersrc_parameters_set failed";
        return;
    }
}

void MultiviewCompositor::make_filter_chain(int idx)
{
    AVFilterInOut* out = avfilter_inout_alloc();
    char label[8];
    snprintf(label, sizeof(label), "v%d", idx);
    out->name = av_strdup(label);
    out->filter_ctx = buffer_ctxs[idx];
    out->pad_idx = 0;
    out->next = outputs;
    outputs = out;
}

bool MultiviewCompositor::initialize(MultiviewCompositorParam& param, shared_ptr<Channel<AVFramePtr>> src, shared_ptr<Channel<AVFramePtr>> dst)
{
    this->param = param;
    this->src = src;
    this->dst = dst;

    make_multiview_compositor();

    filter_graph = avfilter_graph_alloc();
    av_opt_set_int(filter_graph, "threads", 1, 0);

    for(int i = 0; i < 4; i++)
        create_filter_graph(i);

    for(int i = 0; i < 4; i++)
        make_filter_chain(i);

    av_filter_graph_create_filter(&buffersink_ctx, avfilter_get_by_name("buffersink"), "out", nullptr, nullptr, filter_graph);
    
    inputs = avfilter_inout_alloc();
    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    if(avfilter_graph_parse_ptr(filter_graph, filter_descr.c_str(), &inputs, &outputs, nullptr) < 0 )
    {
        BOOST_LOG(error) << "[MULTIVIEW_COMPOSITOR] avfilter_graph_parse_ptr failed";
        return false;
    }

    if(avfilter_graph_config(filter_graph) < 0)
    {
        BOOST_LOG(error) << "[MULTIVIEW_COMPOSITOR] avfilter_graph_config failed";
        return false;
    }

    return true;
}

void MultiviewCompositor::run()
{
    BOOST_LOG(info) << "[MULTIVIEW_COMPOSITOR] run";
    AVFramePtr frame_ptr;
    while(!is_stop && src.get()->pop(frame_ptr))
    {
        auto frame = frame_ptr.get();

        int idx = frame->stream_index;
        int ret = av_buffersrc_add_frame(buffer_ctxs[idx], frame);
        if(ret < 0)
        {
            BOOST_LOG(error) << "[MULTIVIEW_COMPOSITOR] av_buffersrc_add_frame failed";
        }

        while(true)
        {
            AVFramePtr filt_frame_ptr(av_frame_alloc());
            auto filt_frame = filt_frame_ptr.get();

            ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
            if(ret >= 0)
            {
                dst.get()->push(move(filt_frame_ptr));
            }
            else
            {
                if(ret != AVERROR(EAGAIN))
                {
                    BOOST_LOG(error) << "[MULTIVIEW_COMPOSITOR] av_buffersink_get_frame failed";
                }
                break;
            }
        }
    }
    BOOST_LOG(info) << "[MULTIVIEW_COMPOSITOR] stop";
    dst.get()->stop();
}

void MultiviewCompositor::stop()
{
    is_stop = true;
}