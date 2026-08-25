#pragma once

#include "rtsp_types.hpp"
#include "rtsp_frame_parser.hpp"
#include "rtp_thread.hpp"
#include "rtsp_helper.hpp"

#include <cstdint>
#include <string>
#include <expected>
#include <atomic>
#include <thread>
#include <optional>

namespace rtsp_server {
class RtspSession {
  public:
    RtspSession();
    ~RtspSession();

    std::expected<std::string, int> handleEvents(const RtspRequest& rtsp_request);

  private:
    RtspContext rtspContext;
};
}; // namespace rtsp_server
