cmake_minimum_required(VERSION 3.10)
project(hyperstream)

message(STATUS "Start Build")

find_package(PkgConfig REQUIRED)

# 라이브러리 위치 및 컴파일 옵션, 시스템 기본 경로 대신 사용
set(ENV{PKG_CONFIG_PATH} "${CMAKE_SOURCE_DIR}/lib/pkgconfig")

list(APPEND SOURCE_FILE
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/src/Controller.cpp"
    "${CMAKE_SOURCE_DIR}/src/modules/codec/src/VideoDecoder.cpp"
    "${CMAKE_SOURCE_DIR}/src/modules/codec/src/VideoEncoder.cpp"
    "${CMAKE_SOURCE_DIR}/src/modules/ingest/src/MediaSource.cpp"
    "${CMAKE_SOURCE_DIR}/src/modules/processing/src/MultiviewCompositor.cpp"
    "${CMAKE_SOURCE_DIR}/src/modules/transport/src/MediaSink.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/src/Channel.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/src/Logger.cpp"
)

list(APPEND HEADER_FILE
    "${CMAKE_SOURCE_DIR}/src/"
    "${CMAKE_SOURCE_DIR}/src/core/inc"
    "${CMAKE_SOURCE_DIR}/src/modules/codec/inc"
    "${CMAKE_SOURCE_DIR}/src/modules/ingest/inc"
    "${CMAKE_SOURCE_DIR}/src/modules/processing/inc"
    "${CMAKE_SOURCE_DIR}/src/modules/transport/inc"
    "${CMAKE_SOURCE_DIR}/src/utils/inc"
)


set(PKG_CONFIG_USE_STATIC OFF)

pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(SVTAV1ENC REQUIRED SvtAv1Enc)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVFILTER REQUIRED libavfilter)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
pkg_check_modules(SWSCALE REQUIRED libswscale)
pkg_check_modules(VMAF REQUIRED libvmaf)
pkg_check_modules(SRT REQUIRED srt)
find_package(Boost COMPONENTS log REQUIRED) # BOOST는 시스템에 설치 


include_directories(
    ${AVCODEC_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
    ${AVFILTER_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
    ${SWRESAMPLE_INCLUDE_DIRS}
    ${SVTAV1ENC_INCLUDE_DIRS}
    ${SWSCALE_INCLUDE_DIRS}
    ${VMAF_INCLUDE_DIRS}
    ${SRT_INCLUDE_DIRS}
    ${HEADER_FILES}
    ${Boost_INCLUDE_DIRS}
  )

link_directories(
    ${AVCODEC_LIBRARY_DIRS}
    ${AVFORMAT_LIBRARY_DIRS}
    ${AVFILTER_LIBRARY_DIRS}
    ${AVUTIL_LIBRARY_DIRS}
    ${SWRESAMPLE_LIBRARY_DIRS}
    ${SVTAV1ENC_LIBRARY_DIRS}
    ${SWSCALE_LIBRARY_DIRS}
    ${VMAF_LIBRARY_DIRS}
    ${SRT_LIBRARY_DIRS}
    ${Boost_LIBRARY_DIRS}
  )

add_executable(hyperstream ${SOURCE_FILE})

target_link_options(hyperstream PRIVATE "-Wl,--no-as-needed")

target_link_libraries(hyperstream PRIVATE
    ${AVCODEC_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVFILTER_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${SWRESAMPLE_LIBRARIES}
    ${SVTAV1ENC_LIBRARIES}
    ${SWSCALE_LIBRARIES}
    ${VMAF_LIBRARIES}
    ${SRT_LIBRARIES}
    ${Boost_LIBRARIES}
)