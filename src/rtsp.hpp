#ifndef RTSP_HPP
#define RTSP_HPP

#include "rtsp_parser.hpp"

#include <cstdlib>
#include <iostream>
#include <thread>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

using asio::ip::tcp;


class Rtsp {
public:
    Rtsp(asio::io_context& io_context, uint16_t port)
    : _acceptor(io_context, tcp::endpoint(tcp::v4(), port))
    //  _std_out(io_context, ::dup(STDOUT_FILENO))
    {

    }

    void accept(void);

    static void session(tcp::socket sock);

private:
  tcp::acceptor _acceptor;
//   asio::posix::stream_descriptor _std_out;
  enum { max_length = 1024 };
  char data_[max_length];
};

#endif // RTSP_HPP
