#include "rtp_thread.hpp"
#include "rtp_packet.hpp"
#include "video_stream.hpp"
#include "rtsp_types.hpp"
#include <print>
#include <random>
#include <cstdint>
#include <limits>

namespace rtsp_server {
namespace {
constexpr size_t MAX_RTP_NAL_UNIT_SIZE{1400UL};

uint32_t generate_ssrc(void) {
    std::random_device                      rd;
    std::mt19937                            gen(rd());
    std::uniform_int_distribution<uint32_t> distrib(1, std::numeric_limits<uint32_t>::max());
    return distrib(gen);
}
} // namespace
RtpThread::RtpThread(asio::ip::udp::endpoint            clientEndpoint,
                     std::shared_ptr<std::atomic<bool>> isPlaying)
    : m_clientEndpoint(clientEndpoint), m_isPlaying(isPlaying), m_running(true),
      m_serverRtpPort(0) {
    std::println("/// Starting thread ///");
    m_thread = std::thread(&RtpThread::run, this);
}

RtpThread::~RtpThread() {
    std::println("/// ~RTPThread ///");
    stop();
}

void RtpThread::stop(void) {
    std::println("/// Stop called ///");
    m_running.store(false, std::memory_order_release);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void RtpThread::run() {
    using namespace std::chrono_literals;

    std::cout << "Thread started\n";
    asio::io_context      io;                                // local, never needs run()
    asio::ip::udp::socket rtp(io, {asio::ip::udp::v4(), 0}); // server_port
    m_serverRtpPort = rtp.local_endpoint().port();
    std::println("@@@@ server rtp port = [{}] | client rtp port = [{}] @@@@", m_serverRtpPort,
                 static_cast<SocketPort>(m_clientEndpoint.port()));

    uint32_t rtsp_ssrc = generate_ssrc();
    uint16_t rtsp_sequence_number{0U};

    video_stream::VideoStream video_s;
    video_s.setup();
    // auto result = video_s.loop();
    for (const auto video_stream_data : video_s.loop()) {
        // if (nal_units.empty()) { // TODO: or: nal_units[0].empty()
        //     std::println("/// Generator returned empty");
        //     break;
        // }

        // std::println("/// Generator returned 2D vector. size={}", nal_units[0].size());

        if (!m_running.load(std::memory_order_acquire)) {
            std::println("[rtp_thread] thread stop requested.");
            break;
        }

        if (!m_isPlaying->load(std::memory_order_acquire)) {
            std::println("[rtp_thread] pause media.");
            m_isPlaying->wait(false, std::memory_order_acquire);
            std::println("[rtp_thread] replay media.");
        }

        if (video_stream_data.finished) {
            std::println("[rtp_thread] video stream finished.");
            break;
        }

        if (video_stream_data.should_sleep) {
            std::println("[rtp_thread] video stream sleep.");
            std::this_thread::sleep_for(video_stream_data.sleep_time);
        }

        if (video_stream_data.nal_units.size() < 1) {
            break;
        }

        for (const auto& nal_unit : video_stream_data.nal_units) {
            if (!m_running.load(std::memory_order_acquire)) {
                std::println("[rtp_thread] thread stop requested.");
                break;
            }

            RtpPacket rtp_packet{};
            rtp_packet.ssrc = rtsp_ssrc;
            rtp_packet.payload_type = 96U;
            rtp_packet.sequence_number = rtsp_sequence_number;
            rtp_packet.timestamp = video_stream_data.rtp_timestamp;

            if (nal_unit.size() <= MAX_RTP_NAL_UNIT_SIZE) {
                rtp_packet.marker_bit = 1;
                rtp_packet.payload = nal_unit;
                auto buffer_to_send = rtp_packet.serialize();
                rtp.send_to(asio::buffer(buffer_to_send), m_clientEndpoint);
                // rtp.send_to(asio::buffer(rtp_packet.serialize()), m_clientEndpoint);
            } else {
                rtp_packet.marker_bit = 0;
            }

            rtsp_sequence_number++;
        }
    }
    video_s.close();

    /*
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
    */
    std::cout << "Thread stopped\n";
    rtp.send_to(asio::buffer("Bye\r\n"), m_clientEndpoint);
    rtp.close();
}
} // namespace rtsp_server
