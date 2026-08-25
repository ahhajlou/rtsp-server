#include "rtsp_session.hpp"
#include "rtsp_method_parser.hpp"
#include "rtp_thread.hpp"
#include "rtsp_helper.hpp"

#include <asio.hpp>
#include <iostream>
#include <chrono>

namespace rtsp_server {
RtspSession::RtspSession() : isPlaying(std::make_shared<std::atomic<bool>>(false)) {}

RtspSession::~RtspSession() {
    isPlaying->store(false);
    if (rtpThreadCreated) {
        if (rtpThread.has_value()) {
            rtpThread.value().stop();
        }
        // rtpThreadHandle.request_stop();
        // if (rtpThreadHandle.joinable()) {
        //     rtpThreadHandle.join();
        // }
    }
}

std::expected<RtspSessionState, int> RtspSession::changeState(RtspSessionState newState) {
    if (currentState == RtspSessionState::INIT && newState == RtspSessionState::READY) {

    } else if (currentState == RtspSessionState::READY && newState == RtspSessionState::PLAYING) {
        // isPlaying->store(true, std::memory_order_release);

        if (!rtpThreadCreated) {
            // captured at SETUP: client IP from TCP peer + client_port=RTP port
            // asio::ip::udp::endpoint clientEp{ sock.remote_endpoint().address(), rtpPort };
            asio::ip::udp::endpoint clientEp{asio::ip::make_address("127.0.0.1"), 8000};

            rtpThread.emplace(clientEp, isPlaying);
            rtspContext.serverInfo.rtpPort = rtpThread.value().serverRtpPort();
            rtpThreadCreated = true;
        }

        isPlaying->store(true, std::memory_order_release);
        isPlaying->notify_one();

    } else if (currentState == RtspSessionState::PLAYING && newState == RtspSessionState::READY) {
        isPlaying->store(false);

    } else {
        std::cerr << "Invalid state" << std::endl;
        return std::unexpected(-1);
    }

    currentState = newState;

    return currentState;
}

std::expected<std::string, int> RtspSession::handleEvents(const RtspRequest& rtsp_reques) {
    switch (currentState) {
    case RtspSessionState::INIT:
        if (requstFrame.rtspHeaderType == HeaderType::OPTIONS) {
            std::cout << "===> Options\n";
            // (void)changeState(RtspSessionState::INIT);
            (void)OptionMethodParser().parseFrame(rtspContext, requstFrame);
            middlewareProcess(rtspContext);
            return OptionMethodParser().genResponse(rtspContext).value();
            // return RtspSessionState::INIT;
        } else if (requstFrame.rtspHeaderType == HeaderType::DESCRIBE) {
            std::cout << "===> Describe\n";
            // (void)changeState(RtspSessionState::INIT);
            return DescribeMethodParser().genResponse(rtspContext).value();
            // return RtspSessionState::INIT;
        } else if (requstFrame.rtspHeaderType == HeaderType::SETUP) {
            std::cout << "===> Setup\n";
            (void)changeState(RtspSessionState::READY);
            SetupMethodParser().parseFrame(rtspContext, requstFrame).value();
            return SetupMethodParser().genResponse(rtspContext).value();
            // return RtspSessionState::READY;
        } else {
            return std::unexpected(-1);
        }
        break;
    case RtspSessionState::READY:
        if (requstFrame.rtspHeaderType == HeaderType::PLAY) {
            std::cout << "===> Play\n";
            (void)changeState(RtspSessionState::PLAYING);
            return PlayMethodParser().genResponse(rtspContext).value();
            // return RtspSessionState::PLAYING;
        } else {
            return std::unexpected(-1);
        }
        break;
    case RtspSessionState::PLAYING:
        if (requstFrame.rtspHeaderType == HeaderType::PAUSE) {
            // (void)changeState(RtspSessionState::READY);
            std::unexpected(-1);
            // return RtspSessionState::READY;
        } else {
            return std::unexpected(-1);
        }
        break;
    }

    return std::unexpected(-1);
}
}; // namespace rtsp_server
