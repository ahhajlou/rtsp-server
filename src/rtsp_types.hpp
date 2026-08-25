#pragma once

#include "rtp_thread.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

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

struct RtspResponse {
    uint16_t    status{200};
    HeaderMap   headers{};
    std::string body{};

    // Echoes the request's CSeq header verbatim (RFC 2326 12.19).
    std::string serialize(std::string_view cseq) const;
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

std::string_view method_to_string(Method m);
Method           method_from_string(std::string_view s);
std::string_view rtsp_response_reason_phrase(uint16_t status);
RtspResponse     make_error_response(RtspError error, std::string_view cseq);

} // namespace rtsp_server
