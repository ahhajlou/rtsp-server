#include "RTPThread.hpp"

#include <print>

RTPThread::RTPThread(
    asio::ip::udp::endpoint clientEndpoint,
    std::shared_ptr<std::atomic<bool>> isPlaying
) : m_clientEndpoint(clientEndpoint), m_isPlaying(isPlaying), m_running(true), m_serverRtpPort(0)
{
    std::println("/// Starting thread ///");
    m_thread = std::thread(&RTPThread::run, this);
}

RTPThread::~RTPThread()
{
    std::println("/// ~RTPThread ///");
    stop();
}

void RTPThread::stop(void)
{
    std::println("/// Stop called ///");
    m_running.store(false, std::memory_order_release);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void RTPThread::run()
{
    using namespace std::chrono_literals;

    std::cout << "Thread started\n";
    asio::io_context io;                       // local, never needs run()
    asio::ip::udp::socket rtp(io, {asio::ip::udp::v4(), 0}); // server_port
    m_serverRtpPort = rtp.local_endpoint().port();
    std::println("@@@@ rtp port = [{}] @@@@", m_serverRtpPort);

    while (m_running.load(std::memory_order_acquire)) {
        try {
            if (!m_isPlaying->load(std::memory_order_acquire)) {
                std::cout << "stopped\n";
                m_isPlaying->wait(false, std::memory_order_acquire);
            }

        // while (isPlaying->load()) {
            // if (stoken.stop_requested()) {
            //     break;
            // }
            // auto pkt = buildRtpPacket();
            // rtp.send_to(asio::buffer(pkt), clientEp);   // blocking
            // std::this_thread::sleep_for(33ms);

            rtp.send_to(asio::buffer("Hello World\r\n"), m_clientEndpoint);
            std::this_thread::sleep_for(2s); // Sleeps for 2 seconds
        } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
        }
    }

    std::cout << "Thread stopped\n";
    rtp.send_to(asio::buffer("Bye\r\n"), m_clientEndpoint);
    rtp.close();
}
