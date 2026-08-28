#pragma once

#include "rtsp_frame_parser.hpp"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

namespace rtsp_server {
using asio::ip::tcp;

class Rtsp {
  public:
    Rtsp(asio::io_context& io_context, uint16_t port)
        : _acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {}

    void accept(void);

  private:
    tcp::acceptor _acceptor;
};
} // namespace rtsp_server
