#pragma once

#include "rtsp_types.hpp"
#include <asio.hpp>
#include <unordered_map>
#include <expected>

namespace rtsp_server {
struct RtspRequest {
    Method      method{};
    std::string uri{};
    std::string version{};
    HeaderMap   headers{};
    std::string body{}; // Optional
};

std::expected<RtspRequest, RtspError> rtsp_frame_parser(asio::const_buffer buf);
}; // namespace rtsp_server
