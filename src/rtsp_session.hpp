#pragma once

#include "rtsp_parser.hpp"
#include "rtp_thread.hpp"
#include "rtsp_context.hpp"

#include <cstdint>
#include <string>
#include <expected>
#include <atomic>
#include <thread>
#include <optional>

using rtsp_parser::RequestFrame;

enum class RtspSessionState { INIT, READY, PLAYING };

enum class RtspSessionEvent { OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN };

class RtspSession {
  public:
    RtspSession();
    ~RtspSession();

    std::expected<std::string, int> handleEvents(const RequestFrame& requstFrame);

  private:
    std::size_t cseq{};
    std::string sessionId{};
    // std::jthread rtpThreadHandle;
    bool                               rtpThreadCreated{false};
    std::shared_ptr<std::atomic<bool>> isPlaying;
    RtspSessionState                   currentState{RtspSessionState::INIT};

    std::optional<RtpThread> rtpThread;
    RtspContext              rtspContext;

    std::expected<RtspSessionState, int> changeState(RtspSessionState newState);
};
