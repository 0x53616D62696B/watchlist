#pragma once

#include <string>

namespace Watchlist::Messaging {

/// All clients publish request envelopes here; the server subscribes here.
inline constexpr auto RequestsTopic = "watchlist/requests";

/// Per-client acknowledgement topic derived from MqttRequest::sourceClientId.
inline std::string AckTopicForClient(const std::string& clientId)
{
    return "watchlist/acks/" + clientId;
}

} // namespace Watchlist::Messaging
