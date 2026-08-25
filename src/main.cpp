#include "rtsp.hpp"
#include "rtsp_method_parser.hpp"
#include "rtsp_fsm.hpp"

#include <cstdlib>
#include <iostream>
#include <asio.hpp>

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: rtsp-server <port>\n";
            return 1;
        }

        rtsp_server::make_default_transition_table();

        asio::io_context io_context;

        rtsp_server::Rtsp rtsp(io_context, std::atoi(argv[1]));
        rtsp.accept();

        // io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
