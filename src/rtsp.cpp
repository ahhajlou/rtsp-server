#include "rtsp.hpp"
#include "rtsp_types.hpp"
#include "rtsp_frame_parser.hpp"
// #include "rtsp_session.hpp"

namespace rtsp_server {
void Rtsp::accept(void) {
    std::cout << "I am in accept\n";
    for (;;) {
        std::thread(session, _acceptor.accept()).detach();
    }
}

void Rtsp::session(tcp::socket sock) {
    // RtspSession    rtspSession{};
    SessionContext session_context{};

    try {
        for (;;) {
            char data[max_length];
            // std::array<char, max_length> data;

            std::error_code error;

            // TODO: Known limitation, not urgent: read_some() gives you whatever TCP delivered,
            // possibly half a request or two pipelined ones. Robust RTSP needs accumulating
            // into a buffer until \r\n\r\n plus Content-Length bytes
            size_t length = sock.read_some(asio::buffer(data), error);
            if (error == asio::error::eof) {
                std::cout << "Error: Connection closed" << std::endl;
                break; // Connection closed cleanly by peer.
            } else if (error) {
                throw std::system_error(error); // Some other error.
            }

            auto parser_result = rtsp_frame_parser(asio::buffer(data, length));
            if (!parser_result.has_value()) {
                std::cout << "Parse errrrrrorr" << std::endl;
                sock.close();
                return;
            }

            // auto event_result = rtspSession.handleEvents(parser_result.value());
            // if (!event_result.has_value()) {
            //     std::cout << "Handle event error" << std::endl;
            //     sock.close();
            //     return;
            // }

            // std::size_t written = asio::write(sock, asio::buffer(event_result.value()));
            // std::cout << "Written: " << written << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}
} // namespace rtsp_server
