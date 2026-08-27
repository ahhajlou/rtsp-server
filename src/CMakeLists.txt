# set(BINARY ${CMAKE_PROJECT_NAME})

add_library(rtsp_server_lib
    rtp_thread.cpp
    rtsp.cpp
    rtsp_frame_parser.cpp
    rtsp_fsm.cpp
    rtsp_types.cpp
    rtsp_frame_serializer.cpp
    rtsp_connection.cpp
    video_stream.cpp
    rtp_packet.cpp
)

target_include_directories(rtsp_server_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# target_compile_features(rtsp_server_lib PUBLIC cxx_std_23)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavformat libavcodec libavutil libswscale libswresample)
# target_link_libraries(rtsp_server_lib PUBLIC PkgConfig::FFMPEG)
target_link_libraries(rtsp_server_lib PUBLIC asio PkgConfig::FFMPEG)

add_executable(app-rtsp-server main.cpp)
target_link_libraries(app-rtsp-server PRIVATE asio rtsp_server_lib)
