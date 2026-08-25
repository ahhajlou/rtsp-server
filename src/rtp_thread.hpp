#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <asio.hpp>


class RtpThread {
public:
    RtpThread() = delete;
    RtpThread(asio::ip::udp::endpoint clientEndpoint, std::shared_ptr<std::atomic<bool>> isPlaying);
    ~RtpThread();

    RtpThread(const RtpThread&) = delete;
    RtpThread& operator=(const RtpThread&) = delete;
    RtpThread(RtpThread&&) = delete;
    RtpThread& operator=(RtpThread&&) = delete;

    void stop(void);

    uint16_t serverRtpPort(void) {
        return m_serverRtpPort;
    }

private:
    std::thread m_thread;
    asio::ip::udp::endpoint m_clientEndpoint;
    std::shared_ptr<std::atomic<bool>> m_isPlaying;
    std::atomic<bool> m_running;
    uint16_t m_serverRtpPort;

    void run();
};

