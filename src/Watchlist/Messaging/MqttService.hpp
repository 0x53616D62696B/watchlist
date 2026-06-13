#pragma once

#include <functional>
#include <string>

#include "src/Watchlist/AppState.hpp"

namespace Watchlist::Messaging {

/// Transport facade for MQTT IO; currently backed by a local stub until Paho is wired.
class MqttService {
public:
    /// Receives topic and payload from a subscribed MQTT callback.
    using MessageHandler = std::function<void(std::string topic, std::string payload)>;

    explicit MqttService(AppState* appState = nullptr);

    void Connect(std::string host, int port, std::string clientId);
    void Subscribe(std::string topic, MessageHandler handler);
    void Publish(std::string topic, std::string payload);
    void Disconnect();
    [[nodiscard]] bool Connected() const;

private:
    AppState* appState_;
    bool connected_ = false;
    std::string clientId_;
};

} // namespace Watchlist::Messaging
