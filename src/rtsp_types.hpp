#pragma once

#include "rtp_thread.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <thread>

namespace rtsp_server {
using SocketPort = uint16_t;

enum class SessionState { Init, Ready, Playing };

enum class Method { Options, Describe, Setup, Play, Pause, Teardown, Unknown };

enum class RtspError {
    MalformedRequestLine,
    MissingCSeq,
    MalformedHeader,
    UnknownMethod,
    SessionNotFound,
    InvalidStateForMethod, // no transition table entry for (state, method)
    MalformedTransport,
    StreamUnavailable, // guard failure: internal resource not ready
    ParseError,
};

using HeaderMap = std::unordered_map<std::string, std::string>;

struct RtspRequest {
    Method      method{};
    std::string uri{};
    std::string version{};
    HeaderMap   headers{};
    std::string body{}; // Optional
};

struct PeerSocketInfo {
    std::string ip_address{};
    SocketPort  rtp_port{};
    SocketPort  rtcp_port{};
};

struct SessionContext {
    std::size_t  cseq{};
    std::string  session_id{};
    SessionState state{SessionState::Init};

    // Playback bookkeeping
    double npt_position_seconds = 0.0;
    bool   transport_configured = false;

    PeerSocketInfo server_rtp_sockt{};
    PeerSocketInfo client_rtp_sockt{};

    struct RtpThreadT {
        std::optional<RtpThread>           rtp_thread_handle;
        std::shared_ptr<std::atomic<bool>> is_playing;
    } rtp_thread;
};

inline std::string_view method_to_string(Method m) {
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

inline Method method_from_string(std::string_view s) {
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

} // namespace rtsp_server
