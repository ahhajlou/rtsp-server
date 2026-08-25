cmake_minimum_required(VERSION 3.20)

project(rtsp-server VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-38-2
)

FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)

target_include_directories(asio
  INTERFACE ${asio_SOURCE_DIR}/asio/include
)

target_compile_definitions(asio
  INTERFACE
    ASIO_STANDALONE
    ASIO_NO_DEPRECATED
)

target_link_libraries(asio
INTERFACE pthread
)

add_executable(
  ${PROJECT_NAME}
    src/main.cpp
    src/rtsp.cpp
    src/rtsp_frame_parser.cpp
    # src/rtsp_session.cpp
    src/rtp_thread.cpp
    src/rtsp_fsm.cpp
    src/rtsp_method_parser.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE asio)
