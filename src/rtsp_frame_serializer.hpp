#pragma once

#include "rtsp_types.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace rtsp_server {
struct RtspResponse {
    uint16_t    status{200};
    HeaderMap   headers{};
    std::string body{};

    std::string serialize(std::string_view cseq) const;
};

RtspResponse make_error_response(RtspError error, std::string_view cseq);
}; // namespace rtsp_server
