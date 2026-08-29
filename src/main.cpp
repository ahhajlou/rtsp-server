#include "rtsp.hpp"
#include "rtsp_fsm.hpp"

#include <cstdlib>
#include <iostream>
#include <asio.hpp>

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: rtsp-server <port> <video_file_path>\n";
            return 1;
        }

        const std::string video_file_path(argv[2]);

        asio::io_context io_context;

        rtsp_server::Rtsp rtsp(io_context, std::atoi(argv[1]), video_file_path);
        rtsp.accept();

        // io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
