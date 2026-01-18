#include <iostream>
#include "Logger.h"

#include "MediaSource.h"


using namespace std;

int main()
{
    MediaSourceParam param;
    param.url = "srt://0.0.0.0:5000?mode=listener&latency=2000&pkt_size=1316&rcvbuf=1073741824";

    MediaSource* ms = new MediaSource();
    ms->initialize(param, make_unique<Channel<AVPacketPtr>>());
    ms->run();


    BOOST_LOG(debug) << "Test";
}