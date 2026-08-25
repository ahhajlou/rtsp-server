#include "rtsp_method_parser.hpp"

namespace rtsp_server {

std::optional<std::string> OptionMethodParser::parseFrame(SessionContext&    context,
                                                          const RtspRequest& requstFrame) {
    return "";
}

std::optional<std::string> OptionMethodParser::genResponse(const SessionContext& context) {
    // const std::string resp1 =
    //     "RTSP/1.0 200 OK\r\n"
    //     "CSeq: 1\r\n"
    //     "Public: OPTIONS, DESCRIBE, ANNOUNCE, GET_PARAMETER, PAUSE, PLAY, RECORD, SETUP,
    //     SET_PARAMETER, TEARDOWN\r\n" "Server: GStreamer RTSP server\r\n" "Date: Fri, 23 Aug
    //     2026 00:04:52 GMT\r\n\r\n";
    const std::string resp1 =
        std::format("RTSP/1.0 200 OK\r\n"
                    "CSeq: {}\r\n"
                    "Public: OPTIONS, DESCRIBE, ANNOUNCE, GET_PARAMETER, PAUSE, PLAY, RECORD, "
                    "SETUP, SET_PARAMETER, TEARDOWN\r\n"
                    "Server: GStreamer RTSP server\r\n"
                    "Date: {}\r\n\r\n",
                    context.cseq, get_time());

    return resp1;
}

std::optional<std::string> DescribeMethodParser::parseFrame(SessionContext&    context,
                                                            const RtspRequest& requstFrame) {
    return "";
}

std::optional<std::string> DescribeMethodParser::genResponse(const SessionContext& context) {
    const std::string resp1 =
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 2\r\n"
        "Content-Type: application/sdp\r\n"
        "Content-Base: rtsp://127.0.0.1:5000/test/\r\n"
        "Server: GStreamer RTSP server\r\n"
        "Date: Fri, 21 Aug 2026 13:58:52 GMT\r\n"
        "Content-Length: 546\r\n"
        "\r\n"
        "v=0\r\n"
        "o=- 16164648827945584640 1 IN IP4 127.0.0.1\r\n"
        "s=Session streamed with GStreamer\r\n"
        "i=rtsp-server\r\n"
        "t=0 0\r\n"
        "a=tool:GStreamer\r\n"
        "a=type:broadcast\r\n"
        "a=control:*\r\n"
        "a=range:npt=0-106.407\r\n"
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
        "a=mediaclk:sender\r\n"
        "a=ssrc:207104093 cname:user3400444524@host-5fac8c6c\r\n";

    return resp1;
}

std::optional<std::string> SetupMethodParser::parseFrame(SessionContext&    context,
                                                         const RtspRequest& requstFrame) {
    std::println("/// parsing SetupMethodParser ///");
    context.client_rtp_sockt.ip_address = "127.0.0.1";

    auto result = SetupMethodParser::getClientPort(requstFrame);
    if (!result.has_value()) {
        return std::nullopt;
    }
    const auto clientPortsInfo = result.value();
    context.client_rtp_sockt.rtp_port = clientPortsInfo.RtpPort;
    context.client_rtp_sockt.rtcp_port = clientPortsInfo.RtcpPort;

    std::println("/// {} - {} ///", clientPortsInfo.RtpPort, clientPortsInfo.RtcpPort);

    return "";
}

std::optional<std::string> SetupMethodParser::genResponse(const SessionContext& context) {
    // auto res = getClientPort(context.requestHeaderKeyValue);
    // if (!res.has_value()) {
    //     std::println("genResponse error");
    //     return std::unexpected(-1);
    // }
    // auto clientPort = res.value();

    const std::string resp1 = std::format(
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 3\r\n"
        "Transport: "
        "RTP/AVP;unicast;client_port={}-{};server_port={}-{};ssrc=0C58285D;mode=\"PLAY\"\r\n"
        "Server: GStreamer RTSP server\r\n"
        "Session: IL6nZaxJj2DLf9_-\r\n"
        "Date: {}\r\n\r\n",
        context.client_rtp_sockt.rtp_port, context.client_rtp_sockt.rtcp_port,
        context.server_rtp_sockt.rtp_port, context.server_rtp_sockt.rtcp_port, get_time());

    return resp1;
}

std::optional<SetupMethodParser::ClientPortsInfo>
SetupMethodParser::getClientPort(const RtspRequest& requstFrame) {
    std::println("requestHeaderKeyValue.size=[{}]", requstFrame.headers.size());
    auto it = requstFrame.headers.find("Transport");
    if (it == requstFrame.headers.end()) {
        return std::nullopt;
    }

    auto& transport = it->second;

    return SetupMethodParser::parseClientPortInfo(transport);
}

std::optional<SetupMethodParser::ClientPortsInfo>
SetupMethodParser::parseClientPortInfo(std::string_view s) {
    // find "client_port="
    constexpr std::string_view key = "client_port=";
    auto                       pos = s.find(key);
    if (pos == std::string_view::npos)
        return std::nullopt;

    std::string_view rest = s.substr(pos + key.size());

    // rest now looks like "31438-31439" (possibly with trailing chars/params)
    auto dash = rest.find('-');
    if (dash == std::string_view::npos)
        return std::nullopt;

    std::string_view lowSv = rest.substr(0, dash);
    std::string_view highSv = rest.substr(dash + 1);

    ClientPortsInfo result{};
    auto [p1, ec1] = std::from_chars(lowSv.data(), lowSv.data() + lowSv.size(), result.RtpPort);
    if (ec1 != std::errc())
        return std::nullopt;

    auto [p2, ec2] = std::from_chars(highSv.data(), highSv.data() + highSv.size(), result.RtcpPort);
    if (ec2 != std::errc())
        return std::nullopt;

    return result;
}

std::optional<std::string> PlayMethodParser::parseFrame(SessionContext&    context,
                                                        const RtspRequest& requstFrame) {
    return "";
}

std::optional<std::string> PlayMethodParser::genResponse(const SessionContext& context) {
    const std::string resp1 =
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 4\r\n"
        "RTP-Info: url=rtsp://127.0.0.1:5000/test/stream=0;seq=3007;rtptime=1091926906\r\n"
        "Range: npt=0-106.407\r\n"
        "Server: GStreamer RTSP server\r\n"
        "Session: IL6nZaxJj2DLf9_-\r\n"
        "Date: Fri, 21 Aug 2026 13:58:52 GMT\r\n\r\n";

    return resp1;
}
}; // namespace rtsp_server
