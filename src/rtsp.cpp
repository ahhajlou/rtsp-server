#include "rtsp.hpp"
#include "rtsp_types.hpp"
#include "rtsp_fsm.hpp"
#include "rtsp_frame_serializer.hpp"
#include "rtsp_connection.hpp"

namespace rtsp_server {
void Rtsp::accept(void) {
    for (;;) {
        std::thread([](tcp::socket sock) { RtspConnection(std::move(sock)).loop(); },
                    _acceptor.accept())
            .detach();
    }
}
} // namespace rtsp_server
