#include "rtsp_parser.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <ranges>
#include <string_view>

namespace rtsp_parser {
std::expected<RequestFrame, ParseError> parser(asio::const_buffer buf)
{
    const char* data_ptr = static_cast<const char*>(buf.data());
    std::string_view stv = std::string_view(data_ptr);

    std::size_t line_count{0};
    rtsp_parser::RequestFrame rtspRequestLine{};
    for (auto&& line : std::views::split(stv, '\r')) {
        std::string_view sv(line.data(), line.size());
        std::cout << "\n===\n" << sv << "\n===\n" << std::endl;


        if (line_count == 0) {
            auto split_view = sv | std::views::split(' ');
            std::vector<std::string_view> tokens;
            for (auto&& subrange : split_view) {
                tokens.emplace_back(subrange.data(), subrange.size());
            }

            if (tokens.size() <= 3) {
                std::cout << "Parse error" << std::endl;
                return std::unexpected(ParseError::invalid_input);
            }

            if (tokens[0] == "OPTIONS") {
                rtspRequestLine.rtspHeaderType = rtsp_parser::HeaderType::OPTIONS;
            } else if (tokens[0] == "DESCRIBE") {
                rtspRequestLine.rtspHeaderType = rtsp_parser::HeaderType::DESCRIBE;
            } else if (tokens[0] == "SETUP") {
                rtspRequestLine.rtspHeaderType = rtsp_parser::HeaderType::SETUP;
            } else if (tokens[0] == "PLAY") {
                rtspRequestLine.rtspHeaderType = rtsp_parser::HeaderType::PLAY;
            }

            rtspRequestLine.rtspURI = tokens[1];
            rtspRequestLine.rtspVersion = tokens[2];
        } else {

            auto colon_pos = sv.find(':');
            if (colon_pos == std::string_view::npos) {
                // malformed header line — no colon, handle as error
            }

            std::string_view key = sv.substr(0, colon_pos);
            std::string_view value = sv.substr(colon_pos + 1);

            // trim leading whitespace from value (RTSP allows "Key:   value" or "Key: value")
            value.remove_prefix(std::min(value.find_first_not_of(' '), value.size()));

            // trim any trailing \r if you haven't already stripped it during line-splitting
            if (!value.empty() && value.back() == '\r') value.remove_suffix(1);

            rtspRequestLine.rtspRequestHeaderKeyValue.emplace(std::string(key), std::string(value));
        }

        line_count++;
    }
}

std::string genResponse(void)
{
//     const std::string resp =
// R"(RTSP/1.0 200 OK
// CSeq: 1
// Public: OPTIONS, DESCRIBE, ANNOUNCE, GET_PARAMETER, PAUSE, PLAY, RECORD, SETUP, SET_PARAMETER, TEARDOWN
// Server: GStreamer RTSP server
// Date: Fri, 22 Aug 2026 13:58:52 GMT
// )";

    const std::string resp1 =
"RTSP/1.0 200 OK\r\n"
"CSeq: 1\r\n"
"Public: OPTIONS, DESCRIBE, ANNOUNCE, GET_PARAMETER, PAUSE, PLAY, RECORD, SETUP, SET_PARAMETER, TEARDOWN\r\n"
"Server: GStreamer RTSP server\r\n"
"Date: Fri, 23 Aug 2026 00:04:52 GMT\r\n\r\n";

    std::cout << "Response: [ " << resp1 << "] " << std::endl;


    return resp1;
}
} // namespace rtsp_parser
