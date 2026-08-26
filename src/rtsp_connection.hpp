#pragma once

#include "rtsp_types.hpp"
#include "rtp_thread.hpp"
#include <cstdint>
#include <atomic>
#include <memory>
#include <optional>
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

namespace rtsp_server {
using asio::ip::tcp;

class RtspConnection {
  public:
    explicit RtspConnection(tcp::socket sock)
        : m_sock(std::move(sock)), m_is_playing(std::make_shared<std::atomic<bool>>(false)) {}

    RtspConnection(const RtspConnection&) = delete;
    RtspConnection& operator=(const RtspConnection&) = delete;

    RtspConnection(RtspConnection&&) = default;
    RtspConnection& operator=(RtspConnection&&) = default;

    ~RtspConnection() = default;

    void loop(void);

    static constexpr std::size_t max_length{1024};

  private:
    tcp::socket    m_sock;
    SessionContext m_session_context{};

    std::optional<RtpThread>           m_rtp_thread_handle;
    std::shared_ptr<std::atomic<bool>> m_is_playing;
};
} // namespace rtsp_server
