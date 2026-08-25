//
// async_udp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "rtsp.hpp"
#include "rtsp_frame.hpp"

#include <cstdlib>
#include <iostream>
#include <asio.hpp>

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: rtsp-server <port>\n";
            return 1;
        }

        asio::io_context io_context;

        rtsp_server::Rtsp rtsp(io_context, std::atoi(argv[1]));
        rtsp.accept();

        // io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
