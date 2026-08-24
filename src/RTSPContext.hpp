#ifndef RTSP_CONTEXT_HPP
#define RTSP_CONTEXT_HPP

#include "rtsp_parser.hpp"

#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>
#include <format>

inline std::string getTime(void);


struct RTSPContext {
    std::size_t cseq{};
    std::string sessionId{};
    std::string nowTime{};

    struct ServerInfo {
        std::string ipAddress{};
        uint16_t rtpPort{};
        uint16_t rtcpPort{};
    } serverInfo;

    struct ClientInfo {
        std::string ipAddress{};
        uint16_t rtpPort{};
        uint16_t rtcpPort{};
    } clientInfo;
};

inline void middlewareProcess(RTSPContext& rtspContext)
{
    rtspContext.cseq++;
    rtspContext.nowTime = getTime();
}

inline std::string getTime(void)
{
    auto now = std::chrono::system_clock::now();

    // 2. Drop the fractional part by flooring it to whole seconds
    auto whole_seconds = std::chrono::floor<std::chrono::seconds>(now);

    // 3. Format exactly as requested
    std::string formatted = std::format("{:%a, %d %b %Y %T GMT}", whole_seconds);
    return formatted;

    // std::cout << formatted << std::endl;
}

#endif // RTSP_CONTEXT_HPP
