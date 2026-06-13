#include "src/Watchlist/Dispatch/RequestDispatcher.hpp"

#include <format>
#include <utility>

#include "src/Watchlist/Storage/KeyValueStore.hpp"

namespace Watchlist::Dispatch {

RequestDispatcher::RequestDispatcher(
    Concurrency::ThreadPoolManager& runtime,
    std::filesystem::path databasePath,
    AppState* appState,
    AckPublisher ackPublisher)
    : runtime_(runtime)
    , databasePath_(std::move(databasePath))
    , appState_(appState)
    , ackPublisher_(std::move(ackPublisher))
{
}

Messaging::MqttAck RequestDispatcher::HandleIncomingPayload(const std::string& payload)
{
    if (appState_ != nullptr)
        appState_->AddReceived(payload);

    auto parseResult = Messaging::ParseRequest(payload);
    if (!parseResult.request.has_value()) {
        auto ack = Messaging::MakeAck("", "error", parseResult.error);
        if (appState_ != nullptr)
            appState_->AddAck(Messaging::SerializeAck(ack));
        return ack;
    }

    auto request = std::move(*parseResult.request);
    auto future = Dispatch(request);
    return future.get();
}

std::future<Messaging::MqttAck> RequestDispatcher::Dispatch(Messaging::MqttRequest request)
{
    if (appState_ != nullptr) {
        appState_->AddActivity(std::format("Queued {} request {}", Messaging::ToString(request.type), request.id));
    }

    // Handlers may block on device/database/script work, so they run as bounded worker tasks.
    return runtime_.EnqueueTask([this, request = std::move(request)] {
        auto ack = ExecuteWorkerRequest(request);
        if (appState_ != nullptr)
            appState_->AddAck(Messaging::SerializeAck(ack));
        if (ackPublisher_)
            ackPublisher_(request, ack);
        return ack;
    });
}

Messaging::MqttAck RequestDispatcher::ExecuteWorkerRequest(const Messaging::MqttRequest& request)
{
    switch (request.type) {
    case Messaging::MessageType::ExecuteScript:
        return ExecuteScript(request);
    case Messaging::MessageType::SendToDevice:
        return SendToDevice(request);
    case Messaging::MessageType::CmdToDevice:
        return CmdToDevice(request);
    case Messaging::MessageType::StoreDatabaseValue:
        return StoreDatabaseValue(request);
    case Messaging::MessageType::QueryAiPrompt:
        return QueryAiPrompt(request);
    }

    return Messaging::MakeAck(request.id, "error", "unknown message type");
}

Messaging::MqttAck RequestDispatcher::ExecuteScript(const Messaging::MqttRequest& request)
{
    if (request.scriptName != "script1")
        return Messaging::MakeAck(request.id, "error", "script is not allowlisted");

    return Messaging::MakeAck(request.id, "ok", "script1 executed");
}

Messaging::MqttAck RequestDispatcher::SendToDevice(const Messaging::MqttRequest& request)
{
    return Messaging::MakeAck(
        request.id,
        "ok",
        std::format("sent payload to {}", request.deviceId.value_or("device")));
}

Messaging::MqttAck RequestDispatcher::CmdToDevice(const Messaging::MqttRequest& request)
{
    return Messaging::MakeAck(
        request.id,
        "ok",
        std::format("sent command '{}' to {}", request.command.value_or(""), request.deviceId.value_or("device")));
}

Messaging::MqttAck RequestDispatcher::StoreDatabaseValue(const Messaging::MqttRequest& request)
{
    Storage::KeyValueStore store(databasePath_);
    store.Initialize();
    store.Upsert(*request.key, *request.value);
    return Messaging::MakeAck(request.id, "ok", std::format("stored key '{}'", *request.key));
}

Messaging::MqttAck RequestDispatcher::QueryAiPrompt(const Messaging::MqttRequest& request)
{
    return Messaging::MakeAck(request.id, "ok", std::format("AI prompt queued: {}", request.prompt.value_or("")));
}

} // namespace Watchlist::Dispatch
