#include "rtsp_session.hpp"
#include "RTSPFrame.hpp"
#include "RTPThread.hpp"
#include "RTSPContext.hpp"

#include <asio.hpp>
#include <iostream>
#include <chrono>

RtspSession::RtspSession()
    : isPlaying(std::make_shared<std::atomic<bool>>(false))
{

}

RtspSession::~RtspSession()
{
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

std::expected<RtspSessionState, int> RtspSession::changeState(RtspSessionState newState)
{
    if (currentState == RtspSessionState::INIT && newState == RtspSessionState::READY) {

    } else if (currentState == RtspSessionState::READY && newState == RtspSessionState::PLAYING) {
        // isPlaying->store(true, std::memory_order_release);

        if (!rtpThreadCreated) {
            // captured at SETUP: client IP from TCP peer + client_port=RTP port
            // asio::ip::udp::endpoint clientEp{ sock.remote_endpoint().address(), rtpPort };
            asio::ip::udp::endpoint clientEp{ asio::ip::make_address("127.0.0.1"), 8000 };
/*
            rtpThreadHandle = std::jthread([isPlaying = this->isPlaying, clientEp](std::stop_token stoken) {
                using namespace std::chrono_literals;
                std::cout << "Thread started\n";
                asio::io_context io;                       // local, never needs run()
                asio::ip::udp::socket rtp(io, {asio::ip::udp::v4(), 50562}); // server_port

                while (!stoken.stop_requested()) {

                    if (!isPlaying->load(std::memory_order_acquire)) {
                        std::cout << "stopped\n";
                        isPlaying->wait(false, std::memory_order_acquire);
                    }

                // while (isPlaying->load()) {
                    // if (stoken.stop_requested()) {
                    //     break;
                    // }
                    // auto pkt = buildRtpPacket();
                    // rtp.send_to(asio::buffer(pkt), clientEp);   // blocking
                    // std::this_thread::sleep_for(33ms);

                    rtp.send_to(asio::buffer("Hello World\r\n"), clientEp);
                    std::this_thread::sleep_for(2s); // Sleeps for 2 seconds
                }

                std::cout << "Thread stopped\n";
                rtp.send_to(asio::buffer("Bye\r\n"), clientEp);
                rtp.close();
            });

            rtpThreadHandle.detach();
*/

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

std::expected<std::string, int> RtspSession::handleEvents(const RequestFrame& requstFrame)
{
    switch (currentState) {
        case RtspSessionState::INIT:
            if (requstFrame.rtspHeaderType == rtsp_parser::HeaderType::OPTIONS) {
                std::cout << "===> Options\n";
                // (void)changeState(RtspSessionState::INIT);
                (void)OptionFrame().parseFrame(rtspContext, requstFrame);
                middlewareProcess(rtspContext);
                return OptionFrame().genResponse(rtspContext).value();
                // return RtspSessionState::INIT;
            } else if (requstFrame.rtspHeaderType == rtsp_parser::HeaderType::DESCRIBE) {
                std::cout << "===> Describe\n";
                // (void)changeState(RtspSessionState::INIT);
                return DescribeFrame().genResponse(rtspContext).value();
                // return RtspSessionState::INIT;
            } else if (requstFrame.rtspHeaderType == rtsp_parser::HeaderType::SETUP) {
                std::cout << "===> Setup\n";
                (void)changeState(RtspSessionState::READY);
                SetupFrame().parseFrame(rtspContext, requstFrame).value();
                return SetupFrame().genResponse(rtspContext).value();
                // return RtspSessionState::READY;
            } else {
                return std::unexpected(-1);
            }
        break;
        case RtspSessionState::READY:
            if (requstFrame.rtspHeaderType == rtsp_parser::HeaderType::PLAY) {
                std::cout << "===> Play\n";
                (void)changeState(RtspSessionState::PLAYING);
                return PlayFrame().genResponse(rtspContext).value();
                // return RtspSessionState::PLAYING;
            } else {
                return std::unexpected(-1);
            }
        break;
        case RtspSessionState::PLAYING:
            if (requstFrame.rtspHeaderType == rtsp_parser::HeaderType::PAUSE) {
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
