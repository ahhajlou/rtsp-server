#include "rtsp_types.hpp"

namespace rtsp_server {
std::string RtspResponse::serialize(std::string_view cseq) const {
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

    switch (status) {
    case 200:
        return "OK"sv;
    case 400:
        return "Bad Request"sv;
    case 404:
        return "Not Found"sv;
    case 454:
        return "Session Not Found"sv;
    case 455:
        return "Method Not Valid In This State"sv;
    case 461:
        return "Unsupported Transport"sv;
    case 500:
        return "Internal Server Error"sv;
    case 501:
        return "Not Implemented"sv;
    default:
        return "Internal Server Error"sv;
    }
}

RtspResponse make_error_response(RtspError error, std::string_view /*cseq*/) {
    // CSeq is echoed by RtspResponse::serialize, not stored as a header here.
    uint16_t status = 500;
    switch (error) {
    case RtspError::MalformedRequestLine:
    case RtspError::MalformedHeader:
    case RtspError::MissingCSeq:
    case RtspError::ParseError:
        status = 400;
        break;
    case RtspError::UnknownMethod:
        status = 501;
        break;
    case RtspError::SessionNotFound:
        status = 454;
        break;
    case RtspError::InvalidStateForMethod:
        status = 455;
        break;
    case RtspError::MalformedTransport:
        status = 461;
        break;
    case RtspError::StreamUnavailable:
        status = 500;
        break;
    }

    RtspResponse resp{};
    resp.status = status;
    return resp;
}
}; // namespace rtsp_server
