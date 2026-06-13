#include "src/Watchlist/Messaging/MqttService.hpp"

#include <format>
#include <utility>

namespace Watchlist::Messaging {

MqttService::MqttService(AppState* appState)
    : appState_(appState)
{
}

void MqttService::Connect(std::string host, int port, std::string clientId)
{
    clientId_ = std::move(clientId);
    connected_ = true;
    if (appState_ != nullptr) {
        auto console = appState_->SnapshotConsole();
        console.connectionStatus = std::format("Connected to {}:{} as {}", host, port, clientId_);
        appState_->UpdateConsole(console);
        appState_->AddActivity("MQTT service connected using local stub transport");
    }
}

void MqttService::Subscribe(std::string topic, MessageHandler)
{
    if (appState_ != nullptr)
        appState_->AddActivity("Subscribed to " + topic);
}

void MqttService::Publish(std::string topic, std::string payload)
{
    if (appState_ != nullptr)
        appState_->AddActivity(std::format("Published {} bytes to {}", payload.size(), topic));
}

void MqttService::Disconnect()
{
    connected_ = false;
    if (appState_ != nullptr) {
        auto console = appState_->SnapshotConsole();
        console.connectionStatus = "Offline";
        appState_->UpdateConsole(console);
        appState_->AddActivity("MQTT service disconnected");
    }
}

bool MqttService::Connected() const
{
    return connected_;
}

} // namespace Watchlist::Messaging
