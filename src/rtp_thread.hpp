#ifndef RTP_THREAD_HPP
#define RTP_THREAD_HPP

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <asio.hpp>


class RTPThread {
public:
    RTPThread() = delete;
    RTPThread(asio::ip::udp::endpoint clientEndpoint, std::shared_ptr<std::atomic<bool>> isPlaying);
    ~RTPThread();

    RTPThread(const RTPThread&) = delete;
    RTPThread& operator=(const RTPThread&) = delete;
    RTPThread(RTPThread&&) = delete;
    RTPThread& operator=(RTPThread&&) = delete;

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

#endif // RTP_THREAD_HPP
