#include "rtsp_fsm.hpp"
#include "rtsp_helper.hpp"

#include <charconv>
#include <chrono>
#include <format>
#include <string_view>

namespace rtsp_server {

namespace {

RtspResponse base_response(uint16_t status) {
    RtspResponse resp{};
    resp.status = status;
    resp.headers.emplace("Server", "rtsp-server");
    resp.headers.emplace("Date", get_time());
    return resp;
}

}; // namespace

// ---------------------------------------------------------------------
// Request-side method parsing
// ---------------------------------------------------------------------

std::expected<void, RtspError> parse_transport_header(SessionContext&    context,
                                                      const RtspRequest& req) {
    auto it = req.headers.find("Transport");
    if (it == req.headers.end()) {
        return std::unexpected(RtspError::MalformedTransport);
    }

    // Scan the ;-separated params for client_port=<rtp>[-<rtcp>] rather
    // than matching the whole transport string (protocol token varies,
    // e.g. "RTP/AVP/UDP" vs "RTP/AVP").
    constexpr std::string_view key = "client_port=";
    std::string_view           transport{it->second};
    auto                       port_pos = transport.find(key);
    if (port_pos == std::string_view::npos) {
        return std::unexpected(RtspError::MalformedTransport);
    }

    std::string_view ports = transport.substr(port_pos + key.size());
    if (auto param_end = ports.find(';'); param_end != std::string_view::npos) {
        ports = ports.substr(0, param_end);
    }

    auto             dash = ports.find('-');
    std::string_view rtp_sv = (dash == std::string_view::npos) ? ports : ports.substr(0, dash);

    uint16_t rtp_port{0};
    auto [p1, ec1] = std::from_chars(rtp_sv.data(), rtp_sv.data() + rtp_sv.size(), rtp_port);
    if (ec1 != std::errc()) {
        return std::unexpected(RtspError::MalformedTransport);
    }

    // RFC 2326 12.39: when only one port is given, RTCP defaults to RTP+1.
    uint16_t rtcp_port = static_cast<uint16_t>(rtp_port + 1);
    if (dash != std::string_view::npos) {
        std::string_view rtcp_sv = ports.substr(dash + 1);
        auto [p2, ec2] =
            std::from_chars(rtcp_sv.data(), rtcp_sv.data() + rtcp_sv.size(), rtcp_port);
        if (ec2 != std::errc()) {
            return std::unexpected(RtspError::MalformedTransport);
        }
    }

    context.client_rtp_sockt.rtp_port = rtp_port;
    context.client_rtp_sockt.rtcp_port = rtcp_port;
    return {};
}

// ---------------------------------------------------------------------
// Handlers
// ---------------------------------------------------------------------

FsmHandlerReturn handle_options(SessionContext& /*context*/, const RtspRequest& /*req*/) {
    auto resp = base_response(200);
    resp.headers.emplace("Public",
                         "OPTIONS, DESCRIBE, ANNOUNCE, GET_PARAMETER, PAUSE, PLAY, RECORD, SETUP, "
                         "SET_PARAMETER, TEARDOWN");
    return resp;
}

FsmHandlerReturn handle_describe(SessionContext& /*context*/, const RtspRequest& /*req*/) {
    auto resp = base_response(200);
    resp.headers.emplace("Content-Type", "application/sdp");
    // TODO: derive Content-Base / SDP fields from req.uri instead of hardcoding.
    resp.headers.emplace("Content-Base", "rtsp://127.0.0.1:5000/test/");
    resp.body =
        "v=0\r\n"
        "o=- 16164648827945584640 1 IN IP4 127.0.0.1\r\n"
        "s=Session streamed with rtsp-server\r\n"
        "i=rtsp-server\r\n"
        "t=0 0\r\n"
        "a=tool:rtsp-server\r\n"
        "a=type:broadcast\r\n"
        "a=control:*\r\n"
        "a=range:npt=0-\r\n"
        "m=video 0 RTP/AVP 96\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "b=AS:2097\r\n"
        "a=rtpmap:96 H264/90000\r\n"
        "a=framerate:29.970029970029969\r\n"
        "a=fmtp:96 "
        "packetization-mode=1;sprop-parameter-sets=Z0LAKNkAeAIn5cBagICAoAAAfSAAHUwR4wZJ,aMuMsg="
        "=;profile-level-id=42c028;level-asymmetry-allowed=1\r\n"
        "a=control:stream=0\r\n"
        "a=ts-refclk:local\r\n"
        "a=mediaclk:sender\r\n";
    return resp;
}

FsmHandlerReturn handle_setup(SessionContext& context, const RtspRequest& req) {
    auto parsed = parse_transport_header(context, req);
    if (!parsed.has_value()) {
        return std::unexpected(parsed.error());
    }
    context.transport_configured = true;

    if (context.session_id.empty()) {
        // TODO: generate a real unique session id
        context.session_id = "IL6nZaxJj2DLf9_-";
    }

    auto resp = base_response(200);
    resp.headers.emplace("Session", context.session_id);
    resp.headers.emplace(
        "Transport",
        std::format("RTP/AVP;unicast;client_port={}-{};server_port={}-{};ssrc=0C58285D;"
                    "mode=\"PLAY\"",
                    context.client_rtp_sockt.rtp_port, context.client_rtp_sockt.rtcp_port,
                    context.server_rtp_sockt.rtp_port, context.server_rtp_sockt.rtcp_port));
    return resp;
}

FsmHandlerReturn handle_play(SessionContext& context, const RtspRequest& req) {
    bool stream_available = context.transport_configured;
    if (!stream_available) {
        return std::unexpected(RtspError::StreamUnavailable);
    }

    // Media-plane wiring (moved here from the removed RtspSession):
    // create the RTP sender thread lazily, then flip it into playing.
    if (!context.rtp_thread.is_playing) {
        context.rtp_thread.is_playing = std::make_shared<std::atomic<bool>>(false);
    }
    if (!context.rtp_thread.rtp_thread_handle.has_value()) {
        // TODO: take the real client address from the TCP peer endpoint.
        std::string             client_ip = context.client_rtp_sockt.ip_address.empty()
                                                ? "127.0.0.1"
                                                : context.client_rtp_sockt.ip_address;
        asio::ip::udp::endpoint client_ep{asio::ip::make_address_v4(client_ip),
                                          context.client_rtp_sockt.rtp_port};
        context.rtp_thread.rtp_thread_handle.emplace(client_ep, context.rtp_thread.is_playing);
        context.server_rtp_sockt.rtp_port = context.rtp_thread.rtp_thread_handle->serverRtpPort();
    }
    context.rtp_thread.is_playing->store(true, std::memory_order_release);
    context.rtp_thread.is_playing->notify_one();

    auto resp = base_response(200);
    resp.headers.emplace("Session", context.session_id);
    resp.headers.emplace("RTP-Info", std::format("url={};seq=0;rtptime=0", req.uri));
    resp.headers.emplace("Range", "npt=0-");
    return resp;
}

FsmHandlerReturn handle_pause(SessionContext& context, const RtspRequest& /*req*/) {
    if (context.rtp_thread.is_playing) {
        context.rtp_thread.is_playing->store(false, std::memory_order_release);
    }
    auto resp = base_response(200);
    resp.headers.emplace("Session", context.session_id);
    return resp;
}

FsmHandlerReturn handle_teardown(SessionContext& context, const RtspRequest& /*req*/) {
    if (context.rtp_thread.rtp_thread_handle.has_value()) {
        context.rtp_thread.rtp_thread_handle.reset(); // ~RtpThread stops + joins
        context.rtp_thread.is_playing.reset();
    }
    context.transport_configured = false;
    context.npt_position_seconds = 0.0;

    auto resp = base_response(200);
    resp.headers.emplace("Session", context.session_id);
    return resp;
}

// ---------------------------------------------------------------------
// Dispatch one request against a session using the transition table.
// ---------------------------------------------------------------------

FsmHandlerReturn dispatch(const TransitionTable& table, SessionContext& session,
                          const RtspRequest& req) {
    auto it = table.find({session.state, req.method});
    if (it == table.end()) {
        return std::unexpected(RtspError::InvalidStateForMethod);
    }
    const TransitionRule& rule = it->second;
    auto                  result = rule.handle(session, req);
    if (result) {
        session.state = rule.target; // only mutate on success — guard semantics
    }
    return result;
}
}; // namespace rtsp_server
