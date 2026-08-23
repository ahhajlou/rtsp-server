#include "rtsp.hpp"


void RTSP::accept(void)
{
    std::cout << "I am in accept\n";
    for (;;) {
        std::thread(session, _acceptor.accept()).detach();
    }
}

void RTSP::session(tcp::socket sock)
{
    try {
        for (;;) {
            char data[max_length];
            // std::array<char, max_length> data;

            std::error_code error;
            size_t length = sock.read_some(asio::buffer(data), error);
            if (error == asio::error::eof) {
                std::cout << "Error: Connection closed" << std::endl;
                break; // Connection closed cleanly by peer.
            }
            else if (error)
                throw std::system_error(error); // Some other error.

            // asio::write(sock, asio::buffer(data, length));
            // parseBuffer(asio::buffer(data, length));
            rtsp_parser::parser(asio::buffer(data, length));

            // sock.send(asio::buffer(rtsp_parser::genResponse()));
            std::size_t written = asio::write(sock, asio::buffer(rtsp_parser::genResponse()));
            std::cout << "Written: " << written << std::endl;
            // asio::write(sock, asio::buffer("\r\n"));
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}
