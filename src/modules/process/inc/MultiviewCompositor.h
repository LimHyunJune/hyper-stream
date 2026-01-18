#pragma once

#include "IVideoProcessor.h"

#include <string>
#include <map>

using namespace std;

class MultiviewCompositor final : public IVideoProcessor {

private:
    AVFilterGraph* filter_graph = nullptr;
    map<int, AVFilterContext*> buffer_ctxs;
    AVFilterContext* buffersink_ctx = nullptr;

    AVFilterInOut* inputs = nullptr;
    AVFilterInOut* outputs = nullptr;

    string filter_descr = "";

    void make_filter_chain(int idx);
    void make_multiview_compositor();
    void create_filter_graph(int idx);

public:
    MultiviewCompositor() = default;
    ~MultiviewCompositor();

    virtual bool initialize(VideoProcessorParam& param, shared_ptr<Channel<AVFramePtr>> src, shared_ptr<Channel<AVFramePtr>> dst) override;
    virtual void run() override;
    virtual void stop() override;
};