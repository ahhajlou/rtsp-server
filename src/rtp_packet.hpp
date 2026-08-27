#pragma once

#include <cstdint>
#include <vector>

namespace rtsp_server {
/*
https://www.rfc-editor.org/info/rfc6184/

   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |V=2|P|X|  CC   |M|     PT      |       sequence number         |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                           timestamp                           |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |           synchronization source (SSRC) identifier            |
  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
  |            contributing source (CSRC) identifiers             |
  |                             ....                              |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

// Some fields marked as const for now to be distingushable from other filed that is set
struct RtpPacket {
    const uint8_t        version{2U};    // V
    const uint8_t        padding{0U};    // P
    const uint8_t        extension{0U};  // X
    const uint8_t        csrc_count{0U}; // CC
    uint8_t              marker_bit{};   // M
    uint8_t              payload_type{}; // PT
    uint16_t             sequence_number{};
    uint32_t             timestamp{};
    uint32_t             ssrc{};
    std::vector<uint8_t> payload{};

    std::vector<uint8_t> serialize(void) const;
};
} // namespace rtsp_server
