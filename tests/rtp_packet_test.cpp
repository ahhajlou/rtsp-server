#include "rtp_packet.hpp"
#include <cstdint>
#include <gtest/gtest.h>

constexpr uint8_t RTP_PACKET_VERSION{2U};
constexpr uint8_t RTP_PACKET_PADDING{0U};
constexpr uint8_t RTP_PACKET_EXTENSION{0U};
constexpr uint8_t RTP_PACKET_CSRC_COUNT{0U};

class RtpPacketTest : public ::testing::Test {
  protected:
    rtsp_server::RtpPacket rtp_packet{};
};

TEST_F(RtpPacketTest, TestFields) {
    ASSERT_EQ(rtp_packet.version, RTP_PACKET_VERSION);
    ASSERT_EQ(rtp_packet.padding, RTP_PACKET_PADDING);
    ASSERT_EQ(rtp_packet.extension, RTP_PACKET_EXTENSION);
    ASSERT_EQ(rtp_packet.csrc_count, RTP_PACKET_CSRC_COUNT);

    ASSERT_TRUE(rtp_packet.payload.empty());
}

TEST_F(RtpPacketTest, TestSerializerEmptyPayload) {
    const auto buffer = rtp_packet.serialize();
    ASSERT_FALSE(buffer.empty());
    ASSERT_EQ(buffer.size(), 12);
}

TEST_F(RtpPacketTest, TestSerializer) {
    const auto buffer = rtp_packet.serialize();
    ASSERT_GE(buffer.size(), 12);

    const uint8_t first_byte = buffer[0];
    const uint8_t version = (first_byte >> 6);
    const uint8_t padding = ((first_byte >> 5) & 0x01);
    const uint8_t extension = ((first_byte >> 4) & 0x01);
    const uint8_t csrc_count = (first_byte & 0x0F);

    ASSERT_EQ(version, RTP_PACKET_VERSION);
    ASSERT_EQ(padding, RTP_PACKET_PADDING);
    ASSERT_EQ(extension, RTP_PACKET_EXTENSION);
    ASSERT_EQ(csrc_count, RTP_PACKET_CSRC_COUNT);
}

TEST_F(RtpPacketTest, TestSerializerPayload) {
    rtp_packet.payload.emplace_back(0x7F);
    const auto buffer = rtp_packet.serialize();
    ASSERT_EQ(buffer.size(), 13);
}
