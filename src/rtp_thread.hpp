#pragma once

#include "video_stream.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <memory>
#include <asio.hpp>

namespace rtsp_server {
class RtpThread {
  public:
    RtpThread() = delete;
    RtpThread(asio::ip::udp::endpoint clientEndpoint, std::shared_ptr<std::atomic<bool>> isPlaying,
              video_stream::VideoStream video_stream);
    ~RtpThread();

    RtpThread(const RtpThread&) = delete;
    RtpThread& operator=(const RtpThread&) = delete;
    RtpThread(RtpThread&&) = delete;
    RtpThread& operator=(RtpThread&&) = delete;

    void stop(void);

    uint16_t serverRtpPort(void) { return m_serverRtpPort; }

  private:
    std::thread                        m_thread;
    asio::ip::udp::endpoint            m_clientEndpoint;
    std::shared_ptr<std::atomic<bool>> m_isPlaying;
    std::atomic<bool>                  m_running;
    uint16_t                           m_serverRtpPort;
    video_stream::VideoStream          m_video_stream;

    void run();
};
} // namespace rtsp_server
