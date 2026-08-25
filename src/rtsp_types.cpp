#include "rtsp_types.hpp"

namespace rtsp_server {
std::string RtspResponse::serialize(uint64_t cseq) const {
    std::string out = std::format("RTSP/1.0 {} {}\r\nCSeq: {}\r\n", status,
                                  rtsp_response_reason_phrase(status), cseq);
    for (auto& [k, v] : headers)
        out += std::format("{}: {}\r\n", k, v);
    if (!body.empty())
        out += std::format("Content-Length: {}\r\n", body.size());
    return out + "\r\n" + body;
}

std::string_view method_to_string(Method m) {
    using namespace std::literals::string_view_literals;

    switch (m) {
    case Method::Options:
        return "OPTIONS"sv;
    case Method::Describe:
        return "DESCRIBE"sv;
    case Method::Setup:
        return "SETUP"sv;
    case Method::Play:
        return "PLAY"sv;
    case Method::Pause:
        return "PAUSE"sv;
    case Method::Teardown:
        return "TEARDOWN"sv;
    default:
        return "UNKNOWN"sv;
    }
}

Method method_from_string(std::string_view s) {
    if (s == "OPTIONS")
        return Method::Options;
    if (s == "DESCRIBE")
        return Method::Describe;
    if (s == "SETUP")
        return Method::Setup;
    if (s == "PLAY")
        return Method::Play;
    if (s == "PAUSE")
        return Method::Pause;
    if (s == "TEARDOWN")
        return Method::Teardown;
    return Method::Unknown;
}

std::string_view rtsp_response_reason_phrase(uint16_t status) {
    using namespace std::literals::string_view_literals;

    // TODO: complete status code to string conversion
    switch (status) {
    case 200:
        return "OK"sv;
    // TODO: Check default case
    default:
        return ""sv;
    }
}
}; // namespace rtsp_server
