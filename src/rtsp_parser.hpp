#ifndef RTSP_PARSER_HPP
#define RTSP_PARSER_HPP

#include <asio.hpp>
#include <unordered_map>
#include <expected>

namespace rtsp_parser {

enum class parse_error {
    invalid_input
};

enum class HeaderType {
    OPTIONS,
    DESCRIBE,
    SETUP,
    PLAY
};

using RequestHeaderKeyValue = std::unordered_map<std::string, std::string>;

struct RequestLine {
    HeaderType rtspHeaderType;
    std::string rtspURI;
    std::string rtspVersion;
    RequestHeaderKeyValue rtspRequestHeaderKeyValue;
};

std::expected<RequestLine, parse_error> parser(asio::const_buffer buf);
std::string genResponse(void);
}

#endif // RTSP_PARSER_HPP
