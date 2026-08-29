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
    Rtsp(asio::io_context& io_context, uint16_t port, const std::string video_file_path)
        : _acceptor(io_context, tcp::endpoint(tcp::v4(), port)),
          m_video_file_path(video_file_path) {}

    void accept(void);

  private:
    tcp::acceptor     _acceptor;
    const std::string m_video_file_path;
};
} // namespace rtsp_server
