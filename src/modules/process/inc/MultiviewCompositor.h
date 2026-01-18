#pragma once

#include "IVideoProcessor.h"
#include "FilterConfig.h"
#include "Channel.h"
#include "AVMemory.h"

extern "C" {
    #include <libavfilter/avfilter.h>
    #include <libavfilter/buffersrc.h>
    #include <libavfilter/buffersink.h>
    #include <libavutil/opt.h>
    #include <libavutil/pixfmt.h>
}

#include <array>
#include <atomic>
#include <string>
#include <vector>

class MultiviewCompositor final : public IVideoProcessor {
public:
    MultiviewCompositor();
    ~MultiviewCompositor();

    bool initialize(
        FilterConfig& config,
        shared_ptr<Channel<AVFramePtr>> srcChannel,  // 단일 입력 채널
        shared_ptr<Channel<AVFramePtr>> dstChannel) override;
    void run() override;
    void stop() override;

private:
    FilterConfig m_config;

    AVFilterGraph* m_filterGraph = nullptr;
    AVFilterContext* m_sinkContext = nullptr;
    std::vector<AVFilterContext*> m_srcContexts;

    shared_ptr<Channel<AVFramePtr>> m_srcChannel;  // 단일 입력 채널 (4개 프레임이 순차적으로 옴)
    shared_ptr<Channel<AVFramePtr>> m_dstChannel;

    std::atomic<bool> m_running{false};

    bool createFilterGraph();
    bool createBufferSources();
    bool createBufferSink();
    std::string generateFilterString();
    bool configureFilterGraph();
    void releaseFilterGraph();
};
