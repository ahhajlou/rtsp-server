#pragma once

#include "rtsp_types.hpp"
#include "rtsp_frame_parser.hpp"
#include "rtsp_helper.hpp"

#include <print>
#include <string>
#include <expected>
#include <ranges>

namespace rtsp_server {
class RtspMethodParser {
  public:
    RtspMethodParser() {}
    ~RtspMethodParser() = default;

    virtual std::optional<std::string> parseFrame(SessionContext&    context,
                                                  const RtspRequest& requstFrame) = 0;
    virtual std::optional<std::string> genResponse(const SessionContext& context) = 0;
};

class OptionMethodParser : public RtspMethodParser {
  public:
    std::optional<std::string> parseFrame(SessionContext&    context,
                                          const RtspRequest& requstFrame) override;

    std::optional<std::string> genResponse(const SessionContext& context) override;
};

class DescribeMethodParser : public RtspMethodParser {
  public:
    std::optional<std::string> parseFrame(SessionContext&    context,
                                          const RtspRequest& requstFrame) override;

    std::optional<std::string> genResponse(const SessionContext& context) override;
};

class SetupMethodParser : public RtspMethodParser {
  public:
    std::optional<std::string> parseFrame(SessionContext&    context,
                                          const RtspRequest& requstFrame) override;

    std::optional<std::string> genResponse(const SessionContext& context) override;

  private:
    struct ClientPortsInfo {
        uint16_t RtpPort;
        uint16_t RtcpPort;
    };
    static std::optional<ClientPortsInfo> getClientPort(const RtspRequest& requstFrame);
    static std::optional<ClientPortsInfo> parseClientPortInfo(std::string_view s);
};

class PlayMethodParser : public RtspMethodParser {
  public:
    std::optional<std::string> parseFrame(SessionContext&    context,
                                          const RtspRequest& requstFrame) override;

    std::optional<std::string> genResponse(const SessionContext& context) override;
};
}; // namespace rtsp_server
