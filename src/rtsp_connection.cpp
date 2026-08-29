#include "rtsp_connection.hpp"
#include "rtsp_fsm.hpp"
#include "rtsp_frame_serializer.hpp"

namespace rtsp_server {
void RtspConnection::loop(void) {
    try {
        for (;;) {
            char data[1024];
            // TODO: read_some may deliver partial or pipelined frames; a
            // robust server accumulates into a buffer until a complete
            // message (headers + Content-Length body) is available.
            std::error_code error;
            size_t length = m_sock.read_some(asio::buffer(data), error);
            if (error == asio::error::eof) {
                std::cout << "Error: Connection closed" << std::endl;
                break; // Connection closed cleanly by peer.
            } else if (error) {
                throw std::system_error(error); // Some other error.
            }

            auto parser_result = rtsp_frame_parser(asio::buffer(data, length));
            if (!parser_result.has_value()) {
                std::cerr << "Frame parse error: " << static_cast<int>(parser_result.error())
                          << std::endl;
                // Framing is broken, answer 400 and give up on this connection.
                auto out = make_error_response(parser_result.error(), "0").serialize("0");
                asio::write(m_sock, asio::buffer(out));
                m_sock.close();
                return;
            }
            const RtspRequest& req = parser_result.value();

            // Echo the request's CSeq header verbatim (RFC 2326 12.19).
            std::string_view cseq = "0";
            if (auto it = req.headers.find("CSeq"); it != req.headers.end()) {
                cseq = it->second;
            }

            auto event_result = dispatch(TRANSITION_TABLE, m_session_context, req);

            std::string out = event_result.has_value()
                                  ? event_result->serialize(cseq)
                                  : make_error_response(event_result.error(), cseq).serialize(cseq);
            std::size_t written = asio::write(m_sock, asio::buffer(out));
            std::cout << "Written: " << written << std::endl;

            if (req.method == Method::Teardown && event_result.has_value()) {
                m_sock.close();
                return;
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}
} // namespace rtsp_server
