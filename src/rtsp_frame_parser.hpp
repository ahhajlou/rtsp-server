#pragma once

#include "rtsp_types.hpp"
#include <asio.hpp>
#include <unordered_map>
#include <expected>

namespace rtsp_server {
std::expected<RtspRequest, RtspError> rtsp_frame_parser(asio::const_buffer buf);
}; // namespace rtsp_server
