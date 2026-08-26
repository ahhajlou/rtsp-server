#include "rtsp_types.hpp"
#include "rtsp_frame_parser.hpp"
#include <gtest/gtest.h>
#include <asio.hpp>

TEST(RtspFrameParserTest, InvalidRtspPacketRandomStr) {
    auto my_buffer = asio::buffer("Hello World");
    auto result = rtsp_server::rtsp_frame_parser(my_buffer);
    EXPECT_EQ(result.has_value(), false);
}

TEST(RtspFrameParserTest, ValidRtspPacket) {
    auto my_buffer = asio::buffer("OPTIONS rtsp://127.0.0.1:8554/test RTSP/1.0\r\n"
                                  "CSeq: 1\r\n"
                                  "User-Agent: Lavf62.3.100\r\n\r\n");
    auto result = rtsp_server::rtsp_frame_parser(my_buffer);
    EXPECT_EQ(result.has_value(), true);
}

TEST(RtspFrameParserTest, InvalidRtspPacketBadHeader) {
    auto my_buffer = asio::buffer("OPTIONS rtsp://127.0.0.1:8554/test RTSP/1.0\r\n"
                                  "CSeq 1\r\n"
                                  "User-Agent Lavf62.3.100\r\n\r\n");
    auto result = rtsp_server::rtsp_frame_parser(my_buffer);
    EXPECT_EQ(result.has_value(), false);
}
