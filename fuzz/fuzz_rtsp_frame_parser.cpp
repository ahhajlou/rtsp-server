// fuzz/fuzz_rtsp_frame_parser.cpp
#include "rtsp_frame_parser.hpp"
#include <asio.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    auto my_buffer = asio::buffer(data, size);
    rtsp_server::rtsp_frame_parser(my_buffer);
    return 0; // non-zero is reserved for future use — always return 0
}
