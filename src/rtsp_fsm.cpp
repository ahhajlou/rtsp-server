#include "rtsp_fsm.hpp"
#include "rtsp_method_parser.hpp"

namespace rtsp_server {

FsmHandlerReturn handle_options(SessionContext& context, const RtspRequest& req) {
    // OPTIONS never changes state — valid from Init/Ready/Playing alike.
    // (Handled as a self-loop entry per state in the table below.)
    auto parser_result = OptionMethodParser().parseFrame(context, req);
    if (!parser_result.has_value()) {
        return std::unexpected(RtspError::ParseError);
    }

    // middlewareProcess(context);
    OptionMethodParser().genResponse(context).value();

    return SessionState::Init; // overwritten per-entry return value; see table
}

FsmHandlerReturn handle_describe(SessionContext& /*session*/, const RtspRequest& /*req*/) {
    // TODO: build SDP body for req.uri. No state change.
    return SessionState::Init;
}

FsmHandlerReturn handle_setup(SessionContext& session, const RtspRequest& req) {
    auto it = req.headers.find("Transport");
    if (it == req.headers.end()) {
        return std::unexpected(RtspError::MalformedTransport);
    }
    // TODO: parse it->second (protocol, client_port, etc.) into session.
    session.transport_configured = true;
    if (session.session_id.empty()) {
        // TODO: generate a real session id
        session.session_id = "SESSION-0001";
    }
    return SessionState::Ready;
}

FsmHandlerReturn handle_play(SessionContext& session, const RtspRequest& /*req*/) {
    // Guard: this is the case from the earlier discussion — refuse the
    // transition (state stays Ready) if the stream isn't actually available.
    bool stream_available = session.transport_configured; // TODO: real check
    if (!stream_available) {
        return std::unexpected(RtspError::StreamUnavailable);
    }
    return SessionState::Playing;
}

// FsmHandlerReturn handle_play_while_playing(SessionContext& /*session*/,
//                                                                  const RtspRequest& /*req*/) {
//     // Self-transition: repeated PLAY while already Playing is idempotent,
//     // not an error (see conversation: RFC treats this as a no-op 200 OK).
//     return SessionState::Playing;
// }

FsmHandlerReturn handle_pause(SessionContext& /*session*/, const RtspRequest& /*req*/) {
    return SessionState::Ready;
}

FsmHandlerReturn handle_teardown(SessionContext& session, const RtspRequest& /*req*/) {
    session.transport_configured = false;
    session.npt_position_seconds = 0.0;
    return SessionState::Init;
}

// ---------------------------------------------------------------------
// Build the transition table once (e.g. as a static in your server class)
// ---------------------------------------------------------------------

// TransitionTable make_default_transition_table() {
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

// ---------------------------------------------------------------------
// Dispatch one request against a session using the transition table.
// ---------------------------------------------------------------------

FsmHandlerReturn dispatch(const TransitionTable& table, SessionContext& session,
                          const RtspRequest& req) {
    auto it = table.find({session.state, req.method});
    if (it == table.end()) {
        return std::unexpected(RtspError::InvalidStateForMethod);
    }
    auto result = it->second(session, req);
    if (result) {
        session.state = *result; // only mutate on success — guard semantics
    }
    return result;
}
}; // namespace rtsp_server
