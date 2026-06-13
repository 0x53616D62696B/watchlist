#include "src/Watchlist/Messaging/MqttMessages.hpp"

#include <chrono>
#include <format>
#include <regex>
#include <sstream>

namespace Watchlist::Messaging {
namespace {

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

std::string UnescapeJson(std::string value)
{
    std::string unescaped;
    unescaped.reserve(value.size());

    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] != '\\' || index + 1 >= value.size()) {
            unescaped += value[index];
            continue;
        }

        const char escaped = value[++index];
        switch (escaped) {
        case 'n':
            unescaped += '\n';
            break;
        case 'r':
            unescaped += '\r';
            break;
        case 't':
            unescaped += '\t';
            break;
        default:
            unescaped += escaped;
            break;
        }
    }

    return unescaped;
}

std::optional<std::string> JsonStringValue(std::string_view json, std::string_view key)
{
    // The v1 envelope is flat string-only JSON, so a small keyed extractor keeps the dependency surface low.
    const std::regex pattern(std::string("\"") + std::string(key) + R"json("\s*:\s*"((?:\\.|[^"\\])*)")json");
    std::cmatch match;
    const std::string text(json);
    if (!std::regex_search(text.c_str(), match, pattern))
        return std::nullopt;

    return UnescapeJson(match[1].str());
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
    auto typeValue = JsonStringValue(json, "type");
    if (!typeValue.has_value())
        return {.error = "missing required field: type"};

    auto type = MessageTypeFromString(*typeValue);
    if (!type.has_value())
        return {.error = "unknown message type: " + *typeValue};

    MqttRequest request;
    request.type = *type;
    request.id = JsonStringValue(json, "id").value_or("");
    request.createdUtc = JsonStringValue(json, "created_utc").value_or("");
    request.sourceClientId = JsonStringValue(json, "source_client_id").value_or("");
    request.scriptName = JsonStringValue(json, "script_name");
    request.deviceId = JsonStringValue(json, "device_id");
    request.payload = JsonStringValue(json, "payload");
    request.command = JsonStringValue(json, "command");
    request.key = JsonStringValue(json, "key");
    request.value = JsonStringValue(json, "value");
    request.prompt = JsonStringValue(json, "prompt");

    if (const auto missing = MissingRequiredField(request); missing.has_value())
        return {.error = *missing};

    return {.request = std::move(request)};
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
