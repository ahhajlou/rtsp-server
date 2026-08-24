#ifndef RTSP_FRAME_HPP
#define RTSP_FRAME_HPP

#include "rtsp_parser.hpp"
#include "RTSPContext.hpp"

#include <print>
#include <string>
#include <expected>
#include <ranges>


class RTSPFrame {
public:
    RTSPFrame() {}
    ~RTSPFrame() = default;

    virtual std::expected<std::string, int> parseFrame(RTSPContext& rtspContext, const rtsp_parser::RequestFrame& requstFrame) = 0;
    virtual std::expected<std::string, int> genResponse(const RTSPContext& rtspContext) = 0;

};

class OptionFrame: public RTSPFrame {
public:
    std::expected<std::string, int> parseFrame(RTSPContext& rtspContext, const rtsp_parser::RequestFrame& requstFrame) override {
        return "";
    }

    std::expected<std::string, int> genResponse(const RTSPContext& rtspContext) override {
        // const std::string resp1 =
        //     "RTSP/1.0 200 OK\r\n"
        //     "CSeq: 1\r\n"
        //     "Public: OPTIONS, DESCRIBE, ANNOUNCE, GET_PARAMETER, PAUSE, PLAY, RECORD, SETUP, SET_PARAMETER, TEARDOWN\r\n"
        //     "Server: GStreamer RTSP server\r\n"
        //     "Date: Fri, 23 Aug 2026 00:04:52 GMT\r\n\r\n";
        const std::string resp1 = std::format(
            "RTSP/1.0 200 OK\r\n"
            "CSeq: {}\r\n"
            "Public: OPTIONS, DESCRIBE, ANNOUNCE, GET_PARAMETER, PAUSE, PLAY, RECORD, SETUP, SET_PARAMETER, TEARDOWN\r\n"
            "Server: GStreamer RTSP server\r\n"
            "Date: {}\r\n\r\n",
            rtspContext.cseq,
            rtspContext.nowTime
        );

        return resp1;
    }
};

class DescribeFrame: public RTSPFrame {
public:
    std::expected<std::string, int> parseFrame(RTSPContext& rtspContext, const rtsp_parser::RequestFrame& requstFrame) override {
        return "";
    }

    std::expected<std::string, int> genResponse(const RTSPContext& rtspContext) override {
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
            "a=fmtp:96 packetization-mode=1;sprop-parameter-sets=Z0LAKNkAeAIn5cBagICAoAAAfSAAHUwR4wZJ,aMuMsg==;profile-level-id=42c028;level-asymmetry-allowed=1\r\n"
            "a=control:stream=0\r\n"
            "a=ts-refclk:local\r\n"
            "a=mediaclk:sender\r\n"
            "a=ssrc:207104093 cname:user3400444524@host-5fac8c6c\r\n";

        return resp1;
    }
};

class SetupFrame: public RTSPFrame {
public:
    std::expected<std::string, int> parseFrame(RTSPContext& rtspContext, const rtsp_parser::RequestFrame& requstFrame) override {
        std::println("/// parsing SetupFrame ///");
        rtspContext.clientInfo.ipAddress = "127.0.0.1";

        auto result = SetupFrame::getClientPort(requstFrame);
        if (!result.has_value()) {
            return std::unexpected(-1);
        }
        const auto clientPortsInfo = result.value();
        rtspContext.clientInfo.rtpPort = clientPortsInfo.RtpPort;
        rtspContext.clientInfo.rtcpPort = clientPortsInfo.RtcpPort;

        std::println("/// {} - {} ///", clientPortsInfo.RtpPort, clientPortsInfo.RtcpPort);

        return "";
    }

    std::expected<std::string, int> genResponse(const RTSPContext& rtspContext) override {
        // auto res = getClientPort(rtspContext.requestHeaderKeyValue);
        // if (!res.has_value()) {
        //     std::println("genResponse error");
        //     return std::unexpected(-1);
        // }
        // auto clientPort = res.value();

        const std::string resp1 = std::format(
            "RTSP/1.0 200 OK\r\n"
            "CSeq: 3\r\n"
            "Transport: RTP/AVP;unicast;client_port={}-{};server_port={}-{};ssrc=0C58285D;mode=\"PLAY\"\r\n"
            "Server: GStreamer RTSP server\r\n"
            "Session: IL6nZaxJj2DLf9_-\r\n"
            "Date: {}\r\n\r\n",
            rtspContext.clientInfo.rtpPort,
            rtspContext.clientInfo.rtcpPort,
            rtspContext.serverInfo.rtpPort,
            rtspContext.serverInfo.rtcpPort,
            rtspContext.nowTime
        );

        return resp1;
    }

private:
    struct ClientPortsInfo {
        uint16_t RtpPort;
        uint16_t RtcpPort;
    };
    // static std::expected<std::string, int> getClientPort(rtsp_parser::RequestHeaderKeyValue& requestHeaderKeyValue)
    static std::optional<ClientPortsInfo> getClientPort(const rtsp_parser::RequestFrame& requstFrame)
    {
        std::println("requestHeaderKeyValue.size=[{}]", requstFrame.rtspRequestHeaderKeyValue.size());
        auto it = requstFrame.rtspRequestHeaderKeyValue.find("Transport");
        if (it == requstFrame.rtspRequestHeaderKeyValue.end()) {
            return std::nullopt;
        }

        auto& transport = it->second;

        return SetupFrame::parseClientPortInfo(transport);
    }

    static std::optional<ClientPortsInfo> parseClientPortInfo(std::string_view s) {
        // find "client_port="
        constexpr std::string_view key = "client_port=";
        auto pos = s.find(key);
        if (pos == std::string_view::npos) return std::nullopt;

        std::string_view rest = s.substr(pos + key.size());

        // rest now looks like "31438-31439" (possibly with trailing chars/params)
        auto dash = rest.find('-');
        if (dash == std::string_view::npos) return std::nullopt;

        std::string_view lowSv  = rest.substr(0, dash);
        std::string_view highSv = rest.substr(dash + 1);

        ClientPortsInfo result{};
        auto [p1, ec1] = std::from_chars(lowSv.data(), lowSv.data() + lowSv.size(), result.RtpPort);
        if (ec1 != std::errc()) return std::nullopt;

        auto [p2, ec2] = std::from_chars(highSv.data(), highSv.data() + highSv.size(), result.RtcpPort);
        if (ec2 != std::errc()) return std::nullopt;

        return result;
    }
};

class PlayFrame: public RTSPFrame {
public:
    std::expected<std::string, int> parseFrame(RTSPContext& rtspContext, const rtsp_parser::RequestFrame& requstFrame) override {
        return "";
    }

    std::expected<std::string, int> genResponse(const RTSPContext& rtspContext) override {
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
};

#endif // RTSP_FRAME_HPP
