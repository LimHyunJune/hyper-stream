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
        shared_ptr<Channel<AVFramePtr>> srcChannel,  // 단일 입력 채널
        shared_ptr<Channel<AVFramePtr>> dstChannel) = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
};
