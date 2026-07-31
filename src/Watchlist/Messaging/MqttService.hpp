#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <stop_token>
#include <string>

#include "src/Watchlist/AppState.hpp"

namespace Watchlist::Messaging {

/// Thread-safe asynchronous MQTT transport backed by Eclipse Paho.
class MqttService {
public:
    static constexpr int DeliveryQos = 1;
    static constexpr bool RetainMessages = false;

    enum class LifecycleStatus {
        Offline,
        Connecting,
        Connected,
        Reconnecting,
        Disconnecting,
        Error,
    };

    /// Receives topic and payload from a subscribed MQTT callback.
    using MessageHandler = std::function<void(std::string topic, std::string payload)>;
    using StatusHandler = std::function<void(LifecycleStatus status, std::string detail)>;
    using ErrorHandler = std::function<void(std::string error)>;

    explicit MqttService(AppState* appState = nullptr);
    ~MqttService();

    MqttService(const MqttService&) = delete;
    MqttService& operator=(const MqttService&) = delete;

    /// Begin an asynchronous connection attempt. Failures are reported through
    /// SetErrorHandler and Lifecycle(); Run() retries failed initial attempts.
    void Connect(std::string host, int port, std::string clientId);
    void Subscribe(std::string topic, MessageHandler handler);
    void Unsubscribe(std::string topic);
    void Publish(std::string topic, std::string payload);
    /// Wait for outstanding subscribe/unsubscribe/publish callbacks. This is
    /// bounded so a failed broker cannot hang application shutdown forever.
    [[nodiscard]] bool WaitForPendingOperations(std::chrono::milliseconds timeout);
    void Disconnect();
    [[nodiscard]] bool Connected() const;
    [[nodiscard]] LifecycleStatus Lifecycle() const;

    void SetStatusHandler(StatusHandler handler);
    void SetErrorHandler(ErrorHandler handler);

    /// Own the connection lifetime on the caller's runtime-managed service
    /// thread, including stop-aware initial connection retries and shutdown.
    void Run(std::stop_token stopToken, std::string host, int port, std::string clientId);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Watchlist::Messaging
