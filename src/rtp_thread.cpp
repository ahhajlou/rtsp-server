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
            // std::println("[rtp_thread] video stream sleep.");
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

            if (nal_unit.size() <= MAX_RTP_NAL_UNIT_SIZE) {
                RtpPacket rtp_packet{};
                rtp_packet.ssrc = rtsp_ssrc;
                rtp_packet.payload_type = 96U;
                rtp_packet.sequence_number = rtsp_sequence_number++;
                rtp_packet.timestamp = video_stream_data.rtp_timestamp;
                rtp_packet.marker_bit = 1;
                // TODO: Maybe it is better to construct this std::vector at RtpPacket constuction,
                // or use other design t oprevent copy and improve performance
                rtp_packet.payload = std::move(nal_unit);
                auto buffer_to_send = rtp_packet.serialize();
                rtp.send_to(asio::buffer(buffer_to_send), m_clientEndpoint);
            } else {
                // H.264 RFC 6184 FU-A Fragmentation for large NAL units
                const size_t nal_unit_size = nal_unit.size();
                if (nal_unit_size < 2) {
                    continue; // Invalid NAL unit, skip
                }

                const uint8_t nal_header = nal_unit[0];
                const uint8_t nri = nal_header & 0x60; // Keep NRI (Priority)
                const uint8_t original_type =
                    nal_header & 0x1F; // Keep original NAL type (e.g., 1 or 5)

                // FU-A Indicator: F=0, NRI=nri, Type=28 (FU-A)
                const uint8_t fu_indicator = 0x00 | nri | 28;

                // Max payload size per RTP packet.
                // NOTE: Subtract 2 bytes for the FU-A headers.
                // (If MAX_RTP_NAL_UNIT_SIZE is your total MTU limit like 1400,
                // you should actually use: MAX_RTP_NAL_UNIT_SIZE - 12 (RTP header) - 2 (FU-A) =
                // 1386)
                const size_t max_chunk_payload = MAX_RTP_NAL_UNIT_SIZE - 2;

                size_t current_offset = 1; // Start at byte 1 (skip the original NAL header)
                bool   is_first_fragment = true;

                while (current_offset < nal_unit_size) {
                    size_t remaining_bytes = nal_unit_size - current_offset;
                    size_t chunk_size = std::min(remaining_bytes, max_chunk_payload);
                    bool   is_last_fragment = (current_offset + chunk_size == nal_unit_size);

                    RtpPacket rtp_packet{};
                    rtp_packet.ssrc = rtsp_ssrc;
                    rtp_packet.payload_type = 96U;
                    rtp_packet.sequence_number = rtsp_sequence_number++;
                    rtp_packet.timestamp = video_stream_data.rtp_timestamp;

                    // Marker bit is ONLY set on the very last fragment of the NAL unit
                    rtp_packet.marker_bit = is_last_fragment ? 1 : 0;

                    // Pre-allocate to prevent reallocations (Addresses your TODO)
                    std::vector<uint8_t> fu_payload;
                    fu_payload.reserve(2 + chunk_size);

                    // 1. FU Indicator
                    fu_payload.push_back(fu_indicator);

                    // 2. FU Header
                    uint8_t fu_header = 0x00;
                    if (is_first_fragment) {
                        fu_header |= 0x80; // S bit (Start)
                    }
                    if (is_last_fragment) {
                        fu_header |= 0x40; // E bit (End)
                    }
                    fu_header |= original_type; // Original NAL Unit Type
                    fu_payload.push_back(fu_header);

                    // 3. Payload Data (The actual chunk of the NAL unit)
                    fu_payload.insert(fu_payload.end(), nal_unit.begin() + current_offset,
                                      nal_unit.begin() + current_offset + chunk_size);

                    rtp_packet.payload = std::move(fu_payload);

                    rtp.send_to(asio::buffer(rtp_packet.serialize()), m_clientEndpoint);

                    current_offset += chunk_size;
                    is_first_fragment = false;
                }
            }
        }
    }

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
