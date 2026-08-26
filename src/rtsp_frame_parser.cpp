#include "rtsp_frame_parser.hpp"
#include "rtsp_types.hpp"
#include <charconv>
#include <iostream>
#include <vector>
#include <string>
#include <ranges>
#include <string_view>

namespace rtsp_server {

namespace {
constexpr std::string_view trim(std::string_view str) {
    constexpr std::string_view ws = " \t\r\n";
    auto                       start = str.find_first_not_of(ws);
    if (start == std::string_view::npos)
        return {};
    auto end = str.find_last_not_of(ws);
    return str.substr(start, end - start + 1);
}
} // namespace

std::expected<RtspRequest, RtspError> rtsp_frame_parser(asio::const_buffer buf) {
    const char*      data_ptr = static_cast<const char*>(buf.data());
    std::string_view stv = std::string_view(data_ptr, buf.size());

    // Split head/body at the first blank line ("\r\n\r\n", "\n\n" fallback).
    std::size_t sep_pos = stv.find("\r\n\r\n");
    std::size_t sep_len = 4;
    if (sep_pos == std::string_view::npos) {
        sep_pos = stv.find("\n\n");
        sep_len = 2;
    }

    std::string_view head = (sep_pos == std::string_view::npos) ? stv : stv.substr(0, sep_pos);
    std::string_view body =
        (sep_pos == std::string_view::npos) ? std::string_view{} : stv.substr(sep_pos + sep_len);

    std::size_t line_count{0};
    RtspRequest rtsp_request{};
    for (auto&& line : std::views::split(head, '\n')) {
        std::string_view line_sv(line.data(), line.size());

        if (line_sv.size() <= 0) {
            continue;
        }

        line_sv = trim(line_sv);

        if (line_count == 0) {
            auto                          split_view = line_sv | std::views::split(' ');
            std::vector<std::string_view> tokens;
            for (auto&& subrange : split_view) {
                tokens.emplace_back(subrange.data(), subrange.size());
            }

            if (tokens.size() < 3) {
                std::cout << "Parse error. Size: " << tokens.size() << std::endl;
                return std::unexpected(RtspError::MalformedRequestLine);
            }

            rtsp_request.method = method_from_string(tokens[0]);
            rtsp_request.uri = tokens[1];
            rtsp_request.version = tokens[2];
        } else {
            auto colon_pos = line_sv.find(':');
            if (colon_pos == std::string_view::npos) {
                // malformed header line, no colon
                return std::unexpected(RtspError::MalformedRequestLine);
            }

            std::string_view key = line_sv.substr(0, colon_pos);
            std::string_view value = line_sv.substr(colon_pos + 1);

            // trim leading whitespace from value (RTSP allows "Key:   value" or "Key: value")
            value.remove_prefix(std::min(value.find_first_not_of(' '), value.size()));
            key = trim(key);
            value = trim(value);

            rtsp_request.headers.emplace(std::string(key), std::string(value));
        }

        line_count++;
    }

    if (!body.empty()) {
        rtsp_request.body = std::string(body);
    }

    // Validate declared Content-Length against what actually arrived.
    auto cl_it = rtsp_request.headers.find("Content-Length");
    if (cl_it != rtsp_request.headers.end()) {
        std::size_t content_length{0};
        auto [p, ec] = std::from_chars(cl_it->second.data(),
                                       cl_it->second.data() + cl_it->second.size(), content_length);
        if (ec != std::errc() || content_length > rtsp_request.body.size()) {
            return std::unexpected(RtspError::ParseError);
        }
        // Extra bytes beyond Content-Length would belong to a pipelined
        // message — not supported yet, so trim to the declared size.
        rtsp_request.body.resize(content_length);
    }

    return rtsp_request;
}
} // namespace rtsp_server
