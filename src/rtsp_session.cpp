#include "rtsp_session.hpp"


void RtspSession::changeState(RtspSessionState newState)
{
    currentState = newState;
}

// void RtspSession::handleEvents(RtspSessionEvent event)
std::expected<RtspSessionState, int> RtspSession::handleEvents(RtspSessionEvent event)
{
    switch (currentState) {
        case RtspSessionState::INIT:
            if (event == RtspSessionEvent::OPTIONS) {

            }
        break;
        case RtspSessionState::READY:
        break;
        case RtspSessionState::PLAYING:
        break;                
    }

    // return 
}