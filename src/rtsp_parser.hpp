#pragma once

#include <asio.hpp>
#include <unordered_map>
#include <expected>

namespace rtsp_parser {

enum class ParseError {
    invalid_input
};

enum class HeaderType {
    OPTIONS,
    DESCRIBE,
    SETUP,
    PLAY,
    PAUSE,
    TEARDOWN
};

using RequestHeaderKeyValue = std::unordered_map<std::string, std::string>;

struct RequestFrame {
    HeaderType rtspHeaderType;
    std::string rtspURI;
    std::string rtspVersion;
    RequestHeaderKeyValue rtspRequestHeaderKeyValue;
};

std::expected<RequestFrame, ParseError> parser(asio::const_buffer buf);
std::string genResponse(void);
}

