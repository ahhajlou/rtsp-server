#pragma once

#include "rtsp_frame_parser.hpp"
#include "rtsp_types.hpp"

#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>
#include <format>

namespace rtsp_server {
inline std::string get_time(void) {
    auto now = std::chrono::system_clock::now();

    // Drop the fractional part by flooring it to whole seconds
    auto whole_seconds = std::chrono::floor<std::chrono::seconds>(now);

    std::string formatted = std::format("{:%a, %d %b %Y %T GMT}", whole_seconds);
    return formatted;
}
}; // namespace rtsp_server
