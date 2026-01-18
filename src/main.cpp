#include <iostream>
#include "Logger.h"

#include "MediaSource.h"
#include "Controller.h"

using namespace std;

int main()
{
    ControllerParam param;
    param.source_param.url = "srt://0.0.0.0:5000?mode=listener&latency=2000&pkt_size=1316&rcvbuf=1073741824";

    param.decoder_param.hw_device_ctx = Controller::get_instance().get_hw_device_ctx();
    param.decoder_param.hw_frames_ctx = Controller::get_instance().get_hw_frames_ctx();

    param.processor_param.framerate = {60, 1};
    param.processor_param.timebase = {1, 60};
    param.processor_param.width = 3840;
    param.processor_param.height = 2160;
    param.processor_param.hw_frames_ctx = Controller::get_instance().get_hw_frames_ctx();

    param.encoder_param.framerate = {60, 1};
    param.encoder_param.timebase = {1, 60};
    param.encoder_param.width = 3840;
    param.encoder_param.height = 2160;
    param.encoder_param.bit_rate = 10000000;
    param.encoder_param.preset = "p4";
    param.encoder_param.hw_device_ctx = Controller::get_instance().get_hw_device_ctx();
    param.encoder_param.hw_frames_ctx = Controller::get_instance().get_hw_frames_ctx();

    param.sink_param.url = "./output.mp4";

    Controller::get_instance().run();

    BOOST_LOG(debug) << "Test";
}