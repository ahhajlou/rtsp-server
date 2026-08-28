#include "rtp_packet.hpp"
// #include <concepts>
// #include <bit>
#include <cstring>
#include <arpa/inet.h>

namespace rtsp_server {
// template <std::integral T> constexpr T toggle_endian(T value) noexcept {
//     // If the system is little-endian, swap to make it big-endian
//     if constexpr (std::endian::native == std::endian::little) {
//         return std::byteswap(value);
//     }
//     return value; // Already big-endian
// }
std::vector<uint8_t> RtpPacket::serialize(void) const {
    std::vector<uint8_t> buffer(12);
    uint8_t              first{0};
    uint8_t              second{0};

    first |= (version & 0x03) << 6;
    first |= (padding & 0x1) << 5;
    first |= (extension & 0x1) << 4;
    first |= csrc_count & 0x0F;

    second |= (marker_bit & 0x1) << 7;
    second |= payload_type & 0x7F;

    buffer[0] = first;
    buffer[1] = second;

    // const uint16_t sequence_number_be = toggle_endian(sequence_number);
    const uint16_t sequence_number_be = htons(sequence_number);
    std::memcpy(static_cast<void*>(&buffer[2]), static_cast<const void*>(&sequence_number_be),
                sizeof(uint16_t));

    const uint32_t timestamp_be = htonl(timestamp);
    std::memcpy(static_cast<void*>(&buffer[4]), static_cast<const void*>(&timestamp_be),
                sizeof(uint32_t));

    const uint32_t ssrc_be = htonl(ssrc);
    std::memcpy(static_cast<void*>(&buffer[8]), static_cast<const void*>(&ssrc_be),
                sizeof(uint32_t));

    buffer.insert(buffer.end(), payload.begin(), payload.end()); // append payload to buffer

    return buffer;
}
} // namespace rtsp_server
