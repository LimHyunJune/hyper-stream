#pragma once

extern "C" {
    #include <libavutil/pixfmt.h>
}

// 필터 설정 구조체
struct FilterConfig {
    // 입력 영상 크기
    int inputWidth = 3840;
    int inputHeight = 2160;

    // 출력 영상 크기
    int outputWidth = 3840;
    int outputHeight = 2160;

    // 메인 화면 크기
    int mainWidth = 2880;
    int mainHeight = 2160;

    // 서브 화면 크기
    int subWidth = 960;
    int subHeight = 720;

    // 프레임 레이트
    int frameRateNum = 30;
    int frameRateDen = 1;

    // 타임베이스
    int timeBaseNum = 1;
    int timeBaseDen = 30;

    // 픽셀 포맷
    AVPixelFormat pixelFormat = AV_PIX_FMT_NV12;

    // 입력 개수
    int inputCount = 4;
};
