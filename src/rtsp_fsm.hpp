#pragma once

#include "rtsp_types.hpp"
#include "rtsp_frame_parser.hpp"
#include "rtsp_frame_serializer.hpp"
#include <expected>
#include <functional>
#include <map>

namespace rtsp_server {

using FsmHandlerReturn = std::expected<RtspResponse, RtspError>;

// A handler both performs the method-specific work (parsing headers it
// cares about, mutating SessionContext) AND acts as the transition guard:
// returning the new state means "transition succeeded", returning an
// error means "transition refused" — session.state is left untouched by
// the caller in that case.
using Handler = std::function<FsmHandlerReturn(SessionContext&, const RtspRequest&)>;

// One table row = everything about (state, method): the handler doing
// method-specific work, and the state committed on success.
struct TransitionRule {
    Handler      handle;
    SessionState target;
};

// Key = (current state, incoming method). Only pairs present in this map
// are valid transitions; anything missing is InvalidStateForMethod (455).
using TransitionKey = std::pair<SessionState, Method>;

struct TransitionKeyLess {
    bool operator()(const TransitionKey& a, const TransitionKey& b) const {
        return std::tie(a.first, a.second) < std::tie(b.first, b.second);
    }
};

using TransitionTable = std::map<TransitionKey, TransitionRule, TransitionKeyLess>;

// ---------------------------------------------------------------------
// Handlers
// ---------------------------------------------------------------------
// A handler performs the method-specific work (parsing the headers it
// cares about, mutating SessionContext, building a response) AND acts as
// the transition guard: returning a response means "transition
// succeeded", returning an error means "transition refused" —
// session.state is left untouched by dispatch in that case.

FsmHandlerReturn handle_options(SessionContext& context, const RtspRequest& req);

FsmHandlerReturn handle_describe(SessionContext& context, const RtspRequest& req);

FsmHandlerReturn handle_setup(SessionContext& context, const RtspRequest& req);

FsmHandlerReturn handle_play(SessionContext& context, const RtspRequest& req);

FsmHandlerReturn handle_pause(SessionContext& context, const RtspRequest& req);

FsmHandlerReturn handle_teardown(SessionContext& context, const RtspRequest& req);

// Extract client_port / client_rtcp_port from the SETUP "Transport:"
// header into context.client_rtp_sockt. Accepts both "client_port=a-b"
// and the single-port form ("client_port=a" -> rtcp = rtp + 1).
std::expected<void, RtspError> parse_transport_header(SessionContext&    context,
                                                      const RtspRequest& req);

// ---------------------------------------------------------------------
// Transition table: key = (current state, incoming method). Only pairs
// present in this map are valid transitions; anything missing is
// InvalidStateForMethod (455).
// ---------------------------------------------------------------------

// clang-format off
inline const TransitionTable TRANSITION_TABLE = {
    {{SessionState::Init,    Method::Options},  {handle_options,  SessionState::Init}},
    {{SessionState::Init,    Method::Describe}, {handle_describe, SessionState::Init}},
    {{SessionState::Init,    Method::Setup},    {handle_setup,    SessionState::Ready}},
    {{SessionState::Ready,   Method::Play},     {handle_play,     SessionState::Playing}},
    {{SessionState::Playing, Method::Pause},    {handle_pause,    SessionState::Ready}},
    {{SessionState::Init,    Method::Teardown}, {handle_teardown, SessionState::Init}},
    {{SessionState::Ready,   Method::Teardown}, {handle_teardown, SessionState::Init}},
    {{SessionState::Playing, Method::Teardown}, {handle_teardown, SessionState::Init}},
};
// clang-format on

// ---------------------------------------------------------------------
// Dispatch one request against a session using the transition table.
// ---------------------------------------------------------------------

FsmHandlerReturn dispatch(const TransitionTable& table, SessionContext& context,
                          const RtspRequest& req);
} // namespace rtsp_server
