#pragma once

#include <filesystem>
#include <functional>
#include <future>
#include <string>

#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Watchlist/AppState.hpp"
#include "src/Watchlist/Messaging/MqttMessages.hpp"

namespace Watchlist::Dispatch {

/// Publishes a completed acknowledgement to the transport chosen by the app.
using AckPublisher = std::function<void(const Messaging::MqttRequest&, const Messaging::MqttAck&)>;

/// Parses MQTT payloads and routes valid requests onto the bounded worker pool.
class RequestDispatcher {
public:
    RequestDispatcher(
        Concurrency::ThreadPoolManager& runtime,
        std::filesystem::path databasePath,
        AppState* appState,
        AckPublisher ackPublisher);

    /// Validate raw JSON, record it for UI state, and dispatch valid requests.
    [[nodiscard]] Messaging::MqttAck HandleIncomingPayload(const std::string& payload);

    /// Queue a validated request for worker-pool execution and ack publication.
    [[nodiscard]] std::future<Messaging::MqttAck> Dispatch(Messaging::MqttRequest request);

private:
    [[nodiscard]] Messaging::MqttAck ExecuteWorkerRequest(const Messaging::MqttRequest& request);
    [[nodiscard]] Messaging::MqttAck ExecuteScript(const Messaging::MqttRequest& request);
    [[nodiscard]] Messaging::MqttAck SendToDevice(const Messaging::MqttRequest& request);
    [[nodiscard]] Messaging::MqttAck CmdToDevice(const Messaging::MqttRequest& request);
    [[nodiscard]] Messaging::MqttAck StoreDatabaseValue(const Messaging::MqttRequest& request);
    [[nodiscard]] Messaging::MqttAck QueryAiPrompt(const Messaging::MqttRequest& request);

    Concurrency::ThreadPoolManager& runtime_;
    std::filesystem::path databasePath_;
    AppState* appState_;
    AckPublisher ackPublisher_;
};

} // namespace Watchlist::Dispatch
