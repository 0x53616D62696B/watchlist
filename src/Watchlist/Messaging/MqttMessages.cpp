#include "src/Watchlist/Messaging/MqttMessages.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <format>
#include <sstream>

#include "libs/shenanigans/libs/json/single_include/nlohmann/json.hpp"

namespace Watchlist::Messaging {
namespace {

using Json = nlohmann::json;

std::string EscapeJson(std::string_view value)
{
    std::string escaped;
    escaped.reserve(value.size());

    for (const char character : value) {
        switch (character) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped += character;
            break;
        }
    }

    return escaped;
}

std::optional<std::string> JsonStringValue(const Json& json, std::string_view key)
{
    const auto found = json.find(std::string(key));
    if (found == json.end() || !found->is_string())
        return std::nullopt;
    return found->get<std::string>();
}

void AppendField(std::ostringstream& stream, std::string_view name, std::string_view value, bool& first)
{
    if (!first)
        stream << ',';

    first = false;
    stream << '"' << name << "\":\"" << EscapeJson(value) << '"';
}

void AppendOptionalField(std::ostringstream& stream, std::string_view name, const std::optional<std::string>& value, bool& first)
{
    if (value.has_value())
        AppendField(stream, name, *value, first);
}

std::optional<std::string> MissingRequiredField(const MqttRequest& request)
{
    // Keep validation messages stable; unit tests assert these exact strings.
    if (request.id.empty())
        return "missing required field: id";
    if (request.createdUtc.empty())
        return "missing required field: created_utc";
    if (request.sourceClientId.empty())
        return "missing required field: source_client_id";
    if (!IsValidClientId(request.sourceClientId))
        return "invalid field: source_client_id";

    switch (request.type) {
    case MessageType::ExecuteScript:
        if (!request.scriptName.has_value())
            return "missing required field: script_name";
        break;
    case MessageType::SendToDevice:
        if (!request.deviceId.has_value())
            return "missing required field: device_id";
        if (!request.payload.has_value())
            return "missing required field: payload";
        break;
    case MessageType::CmdToDevice:
        if (!request.deviceId.has_value())
            return "missing required field: device_id";
        if (!request.command.has_value())
            return "missing required field: command";
        break;
    case MessageType::StoreDatabaseValue:
        if (!request.key.has_value())
            return "missing required field: key";
        if (!request.value.has_value())
            return "missing required field: value";
        break;
    case MessageType::QueryAiPrompt:
        if (!request.prompt.has_value())
            return "missing required field: prompt";
        break;
    }

    return std::nullopt;
}

std::optional<std::string> NonStringEnvelopeField(const Json& json)
{
    for (const auto field : {
             "id",
             "type",
             "created_utc",
             "source_client_id",
             "script_name",
             "device_id",
             "payload",
             "command",
             "key",
             "value",
             "prompt",
         }) {
        const auto found = json.find(field);
        if (found != json.end() && !found->is_string())
            return std::format("field must be a string: {}", field);
    }
    return std::nullopt;
}

} // namespace

std::string ToString(MessageType type)
{
    switch (type) {
    case MessageType::ExecuteScript:
        return "execute_script";
    case MessageType::SendToDevice:
        return "send_to_device";
    case MessageType::CmdToDevice:
        return "cmd_to_device";
    case MessageType::StoreDatabaseValue:
        return "store_database_value";
    case MessageType::QueryAiPrompt:
        return "query_AI_prompt";
    }

    return "execute_script";
}

std::optional<MessageType> MessageTypeFromString(std::string_view value)
{
    if (value == "execute_script")
        return MessageType::ExecuteScript;
    if (value == "send_to_device")
        return MessageType::SendToDevice;
    if (value == "cmd_to_device")
        return MessageType::CmdToDevice;
    if (value == "store_database_value")
        return MessageType::StoreDatabaseValue;
    if (value == "query_AI_prompt")
        return MessageType::QueryAiPrompt;

    return std::nullopt;
}

bool IsValidClientId(std::string_view value)
{
    if (value.empty() || value.size() > 128)
        return false;

    return std::ranges::all_of(value, [](const unsigned char character) {
        return std::isalnum(character) != 0 || character == '-' || character == '_' || character == '.';
    });
}

std::string SerializeRequest(const MqttRequest& request)
{
    std::ostringstream stream;
    bool first = true;
    stream << '{';
    AppendField(stream, "id", request.id, first);
    AppendField(stream, "type", ToString(request.type), first);
    AppendField(stream, "created_utc", request.createdUtc, first);
    AppendField(stream, "source_client_id", request.sourceClientId, first);
    AppendOptionalField(stream, "script_name", request.scriptName, first);
    AppendOptionalField(stream, "device_id", request.deviceId, first);
    AppendOptionalField(stream, "payload", request.payload, first);
    AppendOptionalField(stream, "command", request.command, first);
    AppendOptionalField(stream, "key", request.key, first);
    AppendOptionalField(stream, "value", request.value, first);
    AppendOptionalField(stream, "prompt", request.prompt, first);
    stream << '}';
    return stream.str();
}

ParseResult ParseRequest(std::string_view json)
{
    ParseResult result;
    const auto object = Json::parse(json, nullptr, false);
    if (object.is_discarded() || !object.is_object()) {
        result.error = "invalid JSON object";
        return result;
    }

    result.requestId = JsonStringValue(object, "id").value_or("");
    result.sourceClientId = JsonStringValue(object, "source_client_id").value_or("");

    if (const auto invalidField = NonStringEnvelopeField(object); invalidField.has_value()) {
        result.error = *invalidField;
        return result;
    }

    auto typeValue = JsonStringValue(object, "type");
    if (!typeValue.has_value()) {
        result.error = "missing required field: type";
        return result;
    }

    auto type = MessageTypeFromString(*typeValue);
    if (!type.has_value()) {
        result.error = "unknown message type: " + *typeValue;
        return result;
    }

    MqttRequest request;
    request.type = *type;
    request.id = result.requestId;
    request.createdUtc = JsonStringValue(object, "created_utc").value_or("");
    request.sourceClientId = result.sourceClientId;
    request.scriptName = JsonStringValue(object, "script_name");
    request.deviceId = JsonStringValue(object, "device_id");
    request.payload = JsonStringValue(object, "payload");
    request.command = JsonStringValue(object, "command");
    request.key = JsonStringValue(object, "key");
    request.value = JsonStringValue(object, "value");
    request.prompt = JsonStringValue(object, "prompt");

    if (const auto missing = MissingRequiredField(request); missing.has_value()) {
        result.error = *missing;
        return result;
    }

    result.request = std::move(request);
    return result;
}

std::string SerializeAck(const MqttAck& ack)
{
    std::ostringstream stream;
    bool first = true;
    stream << '{';
    AppendField(stream, "request_id", ack.requestId, first);
    AppendField(stream, "status", ack.status, first);
    AppendField(stream, "message", ack.message, first);
    AppendField(stream, "completed_utc", ack.completedUtc, first);
    stream << '}';
    return stream.str();
}

MqttAck MakeAck(std::string requestId, std::string status, std::string message)
{
    return {
        .requestId = std::move(requestId),
        .status = std::move(status),
        .message = std::move(message),
        .completedUtc = UtcNowIso8601(),
    };
}

std::string UtcNowIso8601()
{
    return std::format("{:%FT%TZ}", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
}

} // namespace Watchlist::Messaging
