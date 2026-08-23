#ifndef RTSP_SESSION_HPP
#define RTSP_SESSION_HPP

#include <cstdint>
#include <string>
#include <expected>

enum class RtspSessionState {
    INIT,
    READY,
    PLAYING
};

enum class RtspSessionEvent {
    OPTIONS,
    DESCRIBE,
    SETUP,
    PLAY,
    PAUSE,
    TEARDOWN
};

class RtspSession {
public:
    RtspSession() {}

    void changeState(RtspSessionState newState);
    // void handleEvents(RtspSessionEvent event);
    std::expected<RtspSessionState, int> handleEvents(RtspSessionEvent event);
private:
    std::size_t cseq{};
    std::string sessionId{};
    RtspSessionState currentState{RtspSessionState::INIT};
};

#endif // RTSP_SESSION_HPP
