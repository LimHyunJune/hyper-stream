#pragma once

#include "FilterConfig.h"
#include "Channel.h"
#include "AVMemory.h"

#include <memory>
#include <vector>

using namespace std;

class IVideoProcessor {
public:
    IVideoProcessor() = default;
    virtual ~IVideoProcessor() = default;

    virtual bool initialize(
        FilterConfig& config,
        const vector<shared_ptr<Channel<AVFramePtr>>>& srcChannels,
        shared_ptr<Channel<AVFramePtr>> dstChannel) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};
