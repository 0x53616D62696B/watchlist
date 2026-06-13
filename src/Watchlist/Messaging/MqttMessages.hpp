#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace Watchlist::Messaging {

/// Request kinds accepted by the Watchlist MQTT server dispatcher.
enum class MessageType {
    ExecuteScript,
    SendToDevice,
    CmdToDevice,
    StoreDatabaseValue,
    QueryAiPrompt,
};

/// JSON request envelope published by clients to watchlist/requests.
struct MqttRequest {
    std::string id;
    MessageType type = MessageType::ExecuteScript;
    std::string createdUtc;
    std::string sourceClientId;
    std::optional<std::string> scriptName;
    std::optional<std::string> deviceId;
    std::optional<std::string> payload;
    std::optional<std::string> command;
    std::optional<std::string> key;
    std::optional<std::string> value;
    std::optional<std::string> prompt;
};

/// JSON acknowledgement envelope published by the server to a client ack topic.
struct MqttAck {
    std::string requestId;
    std::string status;
    std::string message;
    std::string completedUtc;
};

/// Parsed request or a deterministic validation error.
struct ParseResult {
    std::optional<MqttRequest> request;
    std::string error;
};

[[nodiscard]] std::string ToString(MessageType type);
[[nodiscard]] std::optional<MessageType> MessageTypeFromString(std::string_view value);

/// Serialize and parse the compact JSON envelopes used on MQTT topics.
[[nodiscard]] std::string SerializeRequest(const MqttRequest& request);
[[nodiscard]] ParseResult ParseRequest(std::string_view json);
[[nodiscard]] std::string SerializeAck(const MqttAck& ack);
[[nodiscard]] MqttAck MakeAck(std::string requestId, std::string status, std::string message);
[[nodiscard]] std::string UtcNowIso8601();

} // namespace Watchlist::Messaging
