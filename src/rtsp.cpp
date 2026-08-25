#include "rtsp.hpp"
#include "rtsp_parser.hpp"
#include "rtsp_session.hpp"

namespace rtsp_server {
void Rtsp::accept(void) {
    std::cout << "I am in accept\n";
    for (;;) {
        std::thread(session, _acceptor.accept()).detach();
    }
}

void Rtsp::session(tcp::socket sock) {
    RtspSession rtspSession{};

    try {
        for (;;) {
            char data[max_length];
            // std::array<char, max_length> data;

            std::error_code error;
            size_t          length = sock.read_some(asio::buffer(data), error);
            if (error == asio::error::eof) {
                std::cout << "Error: Connection closed" << std::endl;
                break; // Connection closed cleanly by peer.
            } else if (error) {
                throw std::system_error(error); // Some other error.
            }

            auto result = parser(asio::buffer(data, length));
            if (!result.has_value()) {
                std::cout << "Parse errrrrrorr" << std::endl;
                sock.close();
                return;
            }

            // {
            //     std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;

            //     for (const auto& [key, value] : result.value().rtspRequestHeaderKeyValue) {
            //         std::cout << "\t\t=>" << key << ":" << value << "\n";
            //     }

            //     std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;

            //     {
            //         auto it = result.value().rtspRequestHeaderKeyValue.find("CSeq");
            //         if (it != result.value().rtspRequestHeaderKeyValue.end()) {
            //             auto& entry = it->second;
            //             std::cout <<  "Second === [" << entry << "]" << std::endl;
            //         } else {
            //             std::cout << "Not found\n";
            //         }
            //     }
            // }

            auto eventResult = rtspSession.handleEvents(result.value());
            if (!eventResult.has_value()) {
                std::cout << "Handle event error" << std::endl;
                sock.close();
                return;
            }

            // std::size_t written = asio::write(sock, asio::buffer(genResponse()));
            std::size_t written = asio::write(sock, asio::buffer(eventResult.value()));
            std::cout << "Written: " << written << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}
} // namespace rtsp_server
