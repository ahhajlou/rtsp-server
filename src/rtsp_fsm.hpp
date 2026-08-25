#pragma once

#include "rtsp_types.hpp"
#include <expected>
#include <functional>
#include <map>

namespace rtsp_server {

using FsmHandlerReturn = std::expected<SessionState, RtspError>;

// A handler both performs the method-specific work (parsing headers it
// cares about, mutating SessionContext) AND acts as the transition guard:
// returning the new state means "transition succeeded", returning an
// error means "transition refused" — session.state is left untouched by
// the caller in that case.
using Handler = std::function<FsmHandlerReturn(SessionContext&, const RtspRequest&)>;

// Key = (current state, incoming method). Only pairs present in this map
// are valid transitions; anything missing is InvalidStateForMethod (455).
using TransitionKey = std::pair<SessionState, Method>;

struct TransitionKeyLess {
    bool operator()(const TransitionKey& a, const TransitionKey& b) const {
        return std::tie(a.first, a.second) < std::tie(b.first, b.second);
    }
};

using TransitionTable = std::map<TransitionKey, Handler, TransitionKeyLess>;

// ---------------------------------------------------------------------
// Example handlers
// ---------------------------------------------------------------------
// These are intentionally simple; wire in your real SDP generation,
// transport parsing, and media-availability checks where noted.

FsmHandlerReturn handle_options(SessionContext& context, const RtspRequest& req);

FsmHandlerReturn handle_describe(SessionContext& /*context*/, const RtspRequest& /*req*/);

FsmHandlerReturn handle_setup(SessionContext& context, const RtspRequest& req);

FsmHandlerReturn handle_play(SessionContext& context, const RtspRequest& /*req*/);

FsmHandlerReturn handle_play_while_playing(SessionContext& /*context*/, const RtspRequest& /*req*/);

FsmHandlerReturn handle_pause(SessionContext& /*context*/, const RtspRequest& /*req*/);

FsmHandlerReturn handle_teardown(SessionContext& context, const RtspRequest& /*req*/);

// ---------------------------------------------------------------------
// Build the transition table once (e.g. as a static in your server class)
// ---------------------------------------------------------------------

// // TransitionTable make_default_transition_table();
// consteval TransitionTable make_default_transition_table(void) {
//     TransitionTable t;

//     // OPTIONS / DESCRIBE: valid from any state, always self-loop.
//     for (auto s : {SessionState::Init, SessionState::Ready, SessionState::Playing}) {
//         t[{s, Method::Options}] = [s](SessionContext&, const RtspRequest&) -> FsmHandlerReturn {
//             return s;
//         };
//         t[{s, Method::Describe}] = [s](SessionContext&, const RtspRequest&) -> FsmHandlerReturn {
//             return s;
//         };
//     }

//     t[{SessionState::Init, Method::Setup}] = handle_setup;
//     t[{SessionState::Ready, Method::Setup}] = handle_setup; // adding another track
//     t[{SessionState::Ready, Method::Play}] = handle_play;
//     // t[{SessionState::Playing, Method::Play}] = handle_play_while_playing;
//     t[{SessionState::Playing, Method::Pause}] = handle_pause;

//     // TEARDOWN valid from any state.
//     for (auto s : {SessionState::Init, SessionState::Ready, SessionState::Playing}) {
//         t[{s, Method::Teardown}] = handle_teardown;
//     }

//     return t;
// }

// clang-format off
inline const TransitionTable TRANSITION_TABLE = {
    {{SessionState::Init,       Method::Options},       handle_options},
    {{SessionState::Init,       Method::Describe},      handle_describe},
    {{SessionState::Init,       Method::Setup},         handle_setup},
    {{SessionState::Ready,      Method::Play},          handle_play},
    {{SessionState::Playing,    Method::Pause},         handle_pause},
    {{SessionState::Init,       Method::Teardown},      handle_teardown},
    {{SessionState::Ready,      Method::Teardown},      handle_teardown},
    {{SessionState::Playing,    Method::Teardown},      handle_teardown},
};
// clang-format on

// ---------------------------------------------------------------------
// Dispatch one request against a session using the transition table.
// ---------------------------------------------------------------------

FsmHandlerReturn dispatch(const TransitionTable& table, SessionContext& context,
                          const RtspRequest& req);
}; // namespace rtsp_server
