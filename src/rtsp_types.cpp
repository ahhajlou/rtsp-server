#include "rtsp_types.hpp"

namespace rtsp_server {
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
}; // namespace rtsp_server
