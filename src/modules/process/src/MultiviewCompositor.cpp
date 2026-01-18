#include "MultiviewCompositor.h"

extern "C" {
    #include <libavutil/pixdesc.h>
}

MultiviewCompositor::MultiviewCompositor() {}

MultiviewCompositor::~MultiviewCompositor() { stop(); releaseFilterGraph(); }

bool MultiviewCompositor::initialize(
    // 화면 크기, fps 등 설정값 저장
    FilterConfig& config,
    const vector<shared_ptr<Channel<AVFramePtr>>>& srcChannels,
    shared_ptr<Channel<AVFramePtr>> dstChannel)
{
    m_config = config;
    if ((int)srcChannels.size() != m_config.inputCount) return false;

    m_srcChannels = srcChannels; // 입력 채널 저장
    m_dstChannel = dstChannel; // 출력 채널 저장
    if (!m_dstChannel) return false; // 출력 채널 유효성 검사

    // 버퍼 공간 확보
    m_srcContexts.resize(m_config.inputCount, nullptr);

    // 필터 그래프 생성
    return createFilterGraph() && createBufferSources() &&
           createBufferSink() && configureFilterGraph();
}

bool MultiviewCompositor::createFilterGraph()
{
    //영상 필터 공간 생성
    m_filterGraph = avfilter_graph_alloc();
    return m_filterGraph != nullptr;
}

bool MultiviewCompositor::createBufferSources()
{
    // buffer 소스 필터 생성 (입력 영상용)
    const AVFilter* buf = avfilter_get_by_name("buffer");
    if (!buf) return false;

    // 4개 버퍼 생성 (for문)
    for (int i = 0; i < m_config.inputCount; ++i) {
        char args[256];
        // 버퍼 설정값 만들기
        // 설정	        의미	    예시
        // video_size	영상 크기	1920x1080
        // pix_fmt	    픽셀 형식	YUV420 등
        // time_base	시간 단위	1/90000
        // frame_rate	프레임 속도	30/1 (30fps)

        snprintf(args, sizeof(args),
            "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:frame_rate=%d/%d",
            m_config.inputWidth, m_config.inputHeight, (int)m_config.pixelFormat,
            m_config.timeBaseNum, m_config.timeBaseDen,
            m_config.frameRateNum, m_config.frameRateDen);

        char name[8];
        snprintf(name, sizeof(name), "in%d", i);

        // 필터 생성 및 그래프에 연결
        if (avfilter_graph_create_filter(&m_srcContexts[i], buf, name, args, nullptr, m_filterGraph) < 0)
            return false;
    }
    return true;
}

bool MultiviewCompositor::createBufferSink()
{
    // buffer 싱크 필터 생성 (출력 영상용)
    const AVFilter* sink = avfilter_get_by_name("buffersink");
    // 싱크 필터 가져오기
    if (!sink) return false;

    // 싱크 필터 생성 및 그래프에 연결
    if (avfilter_graph_create_filter(&m_sinkContext, sink, "out", nullptr, nullptr, m_filterGraph) < 0)
        return false;
        
    // 싱크 필터의 픽셀 포맷 설정
    AVPixelFormat fmts[] = { m_config.pixelFormat, AV_PIX_FMT_NONE };

    // 지원하는 픽셀 포맷 목록 설정
    av_opt_set_int_list(m_sinkContext, "pix_fmts", fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    return true;
}

std::string MultiviewCompositor::generateFilterString()
{
    // 필터 문자열 생성 // 4분할 화면 구성
    //┌─────────────────────┬─────────┐
    //│                     │  sub1   │  ← in1 (subHeight)
    //│        main         ├─────────┤
    //│        (in0)        │  sub2   │  ← in2 (subHeight)
    //│                     ├─────────┤
    //│                     │  sub3   │  ← in3 (subHeight)
    //└─────────────────────┴─────────┘
    //      mainWidth        subWidth

    
    const char* fmt = av_get_pix_fmt_name(m_config.pixelFormat);
    auto& c = m_config;

    char buf[1024];
    snprintf(buf, sizeof(buf),
        "[in0]scale=%d:%d,format=%s[main];" // in0을 메인 크기로 조절 → [main]
        "[in1]scale=%d:%d,format=%s[sub1];" // in1을 작은 크기로 조절 → [sub1]
        "[in2]scale=%d:%d,format=%s[sub2];" // 	in2 조절 → [sub2]
        "[in3]scale=%d:%d,format=%s[sub3];" // in3 조절 → [sub3]
        "color=c=black:s=%dx%d:r=%d,format=%s[bg];" // 검은 배경 생성 → [bg]
        "[bg][main]overlay=0:0[v1];" //   → 배경 위에 main을 (0,0) 위치에 올림
        "[v1][sub1]overlay=%d:0[v2];"// → v1 위에 sub1을 (mainWidth, 0)에 올림
        "[v2][sub2]overlay=%d:%d[v3];" //    → v2 위에 sub2를 (mainWidth, subHeight)에 올림
        "[v3][sub3]overlay=%d:%d[out]", //  → v3 위에 sub3를 (mainWidth, subHeight×2)에 올림
        c.mainWidth, c.mainHeight, fmt,
        c.subWidth, c.subHeight, fmt,
        c.subWidth, c.subHeight, fmt,
        c.subWidth, c.subHeight, fmt,
        c.outputWidth, c.outputHeight, c.frameRateNum, fmt,
        c.mainWidth,
        c.mainWidth, c.subHeight,
        c.mainWidth, c.subHeight * 2);

    return std::string(buf);
}

bool MultiviewCompositor::configureFilterGraph()
{
    // 필터 그래프 구성 
    // 입력을 필터 그래프에 연결

    AVFilterInOut *inputs = nullptr, *outputs = nullptr;

    for (int i = m_config.inputCount - 1; i >= 0; --i) {
        AVFilterInOut* in = avfilter_inout_alloc();
        char name[8];
        snprintf(name, sizeof(name), "in%d", i);
        in->name = av_strdup(name); // 입력 이름 설정 
        in->filter_ctx = m_srcContexts[i];  
        in->pad_idx = 0; 
        in->next = inputs; 
        inputs = in;
    }

    outputs = avfilter_inout_alloc();
    outputs->name = av_strdup("out");
    outputs->filter_ctx = m_sinkContext; // 출력 버퍼 
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    std::string filterStr = generateFilterString(); 
    int ret = avfilter_graph_parse_ptr(m_filterGraph, filterStr.c_str(), &outputs, &inputs, nullptr);
    avfilter_inout_free(&inputs); // 임시 연결 해제
    avfilter_inout_free(&outputs);

    return ret >= 0 && avfilter_graph_config(m_filterGraph, nullptr) >= 0;
}

void MultiviewCompositor::run()
{
    m_running = true;
    // 영상을 받고 내보내기
    while (m_running) {
        // 입력 투입 (pop은 데이터 올 때까지 대기)
        for (int i = 0; i < m_config.inputCount && m_running; ++i) {
            AVFramePtr frame = m_srcChannels[i]->pop(); //  // 채널에서 영상 꺼내기 (기다림)
            if (frame)
                 // 필터에 영상 넣기
                av_buffersrc_add_frame_flags(m_srcContexts[i], frame.get(), AV_BUFFERSRC_FLAG_KEEP_REF);
        }

        // 출력 추출
        AVFrame* out;
        while (m_running && (out = av_frame_alloc())) {
            if (av_buffersink_get_frame(m_sinkContext, out) < 0) { 
                // 출력 없으면 메모리 해제하고 탈출
                av_frame_free(&out); 
                break; 
            }
            // 합쳐진 결과를 다음 단계로 전달
            m_dstChannel->push(AVFramePtr(out));
        }
    }
}

void MultiviewCompositor::stop() { 
    // 멈추기
    m_running = false; 
}

void MultiviewCompositor::releaseFilterGraph()
{   // 정리하기
    if (m_filterGraph) { avfilter_graph_free(&m_filterGraph); m_filterGraph = nullptr; }
    m_srcContexts.clear();
    m_sinkContext = nullptr;
}
