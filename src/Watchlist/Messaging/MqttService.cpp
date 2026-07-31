#include "src/Watchlist/Messaging/MqttService.hpp"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <format>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include <mqtt/async_client.h>

namespace Watchlist::Messaging {
namespace {

using namespace std::chrono_literals;

std::string StatusText(MqttService::LifecycleStatus status)
{
    switch (status) {
    case MqttService::LifecycleStatus::Offline:
        return "Offline";
    case MqttService::LifecycleStatus::Connecting:
        return "Connecting";
    case MqttService::LifecycleStatus::Connected:
        return "Connected";
    case MqttService::LifecycleStatus::Reconnecting:
        return "Reconnecting";
    case MqttService::LifecycleStatus::Disconnecting:
        return "Disconnecting";
    case MqttService::LifecycleStatus::Error:
        return "Error";
    }
    return "Unknown";
}

bool TopicMatches(std::string_view filter, std::string_view topic)
{
    while (true) {
        const auto filterSeparator = filter.find('/');
        const auto topicSeparator = topic.find('/');
        const auto filterLevel = filter.substr(0, filterSeparator);
        const auto topicLevel = topic.substr(0, topicSeparator);

        if (filterLevel == "#")
            return filterSeparator == std::string_view::npos;
        if (filterLevel != "+" && filterLevel != topicLevel)
            return false;

        const bool filterEnded = filterSeparator == std::string_view::npos;
        const bool topicEnded = topicSeparator == std::string_view::npos;
        if (filterEnded || topicEnded)
            return filterEnded && topicEnded;

        filter.remove_prefix(filterSeparator + 1);
        topic.remove_prefix(topicSeparator + 1);
    }
}

} // namespace

class MqttService::Impl final : public mqtt::callback, public mqtt::iaction_listener {
    class OperationListener final : public mqtt::iaction_listener {
    public:
        OperationListener(Impl& owner, std::string operation)
            : owner_(owner)
            , operation_(std::move(operation))
        {
        }

        void on_failure(const mqtt::token& token) override
        {
            owner_.CompleteOperation(*this, std::format(
                                                "{} failed: {} (code {})",
                                                operation_,
                                                token.get_error_message(),
                                                token.get_return_code()));
        }

        void on_success(const mqtt::token&) override
        {
            owner_.CompleteOperation(*this, {});
        }

    private:
        Impl& owner_;
        std::string operation_;
    };

public:
    explicit Impl(AppState* appState)
        : appState_(appState)
    {
    }

    ~Impl() override
    {
        Disconnect();
    }

    void Connect(std::string host, int port, std::string clientId)
    {
        if (host.empty()) {
            ReportError("MQTT broker host must not be empty", true);
            return;
        }
        if (port < 1 || port > 65535) {
            ReportError(std::format("MQTT broker port {} is outside 1-65535", port), true);
            return;
        }
        if (clientId.empty()) {
            ReportError("MQTT client ID must not be empty", true);
            return;
        }

        std::scoped_lock operationLock(operationMutex_);
        {
            std::scoped_lock lock(mutex_);
            if (status_ == LifecycleStatus::Connecting || status_ == LifecycleStatus::Connected
                || status_ == LifecycleStatus::Reconnecting) {
                return;
            }
            shuttingDown_ = false;
        }

        ReleaseClient();
        SetStatus(LifecycleStatus::Connecting, std::format("Connecting to {}:{} as {}", host, port, clientId));

        try {
            const auto createOptions = mqtt::create_options_builder()
                                           .send_while_disconnected(true, true)
                                           .max_buffered_messages(100)
                                           .delete_oldest_messages(false)
                                           .finalize();
            auto client = std::make_shared<mqtt::async_client>(
                std::format("tcp://{}:{}", host, port), clientId, createOptions);
            client->set_callback(*this);

            const auto connectOptions = mqtt::connect_options_builder()
                                            .clean_session(true)
                                            .keep_alive_interval(20s)
                                            .automatic_reconnect(1s, 30s)
                                            .finalize();
            {
                std::scoped_lock lock(mutex_);
                client_ = client;
            }
            client->connect(connectOptions, nullptr, *this);
        }
        catch (const mqtt::exception& exception) {
            ReportError(std::format("MQTT connection attempt failed: {}", exception.what()), true);
        }
        catch (const std::exception& exception) {
            ReportError(std::format("Unable to create MQTT transport: {}", exception.what()), true);
        }
    }

    void Subscribe(std::string topic, MessageHandler handler)
    {
        if (topic.empty()) {
            ReportError("Cannot subscribe to an empty MQTT topic", false);
            return;
        }
        if (!handler) {
            ReportError(std::format("Cannot subscribe to {} without a message handler", topic), false);
            return;
        }

        std::shared_ptr<mqtt::async_client> client;
        bool connected = false;
        {
            std::scoped_lock lock(mutex_);
            subscriptions_.insert_or_assign(topic, std::move(handler));
            client = client_;
            connected = status_ == LifecycleStatus::Connected;
        }
        if (connected)
            SubscribeClient(client, topic);
    }

    void Unsubscribe(std::string topic)
    {
        if (topic.empty()) {
            ReportError("Cannot unsubscribe from an empty MQTT topic", false);
            return;
        }

        std::shared_ptr<mqtt::async_client> client;
        bool connected = false;
        {
            std::scoped_lock lock(mutex_);
            subscriptions_.erase(topic);
            client = client_;
            connected = status_ == LifecycleStatus::Connected;
        }
        if (!connected || !client)
            return;

        try {
            auto listener = TrackOperation("MQTT unsubscribe from " + topic);
            try {
                client->unsubscribe(topic, nullptr, *listener);
            }
            catch (...) {
                CancelOperation(*listener);
                throw;
            }
        }
        catch (const mqtt::exception& exception) {
            ReportError(std::format("MQTT unsubscribe from {} failed: {}", topic, exception.what()), false);
        }
    }

    void Publish(std::string topic, std::string payload)
    {
        if (topic.empty()) {
            ReportError("Cannot publish to an empty MQTT topic", false);
            return;
        }

        std::shared_ptr<mqtt::async_client> client;
        {
            std::scoped_lock lock(mutex_);
            client = client_;
        }
        if (!client) {
            ReportError(std::format("Cannot publish to {} before Connect", topic), false);
            return;
        }

        try {
            auto listener = TrackOperation("MQTT publish to " + topic);
            auto message = mqtt::make_message(std::move(topic), std::move(payload), DeliveryQos, RetainMessages);
            try {
                client->publish(std::move(message), nullptr, *listener);
            }
            catch (...) {
                CancelOperation(*listener);
                throw;
            }
        }
        catch (const mqtt::exception& exception) {
            ReportError(std::format("MQTT publish failed: {}", exception.what()), false);
        }
    }

    bool WaitForPendingOperations(std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(mutex_);
        return pendingOperationsChanged_.wait_for(lock, timeout, [this] { return pendingOperations_.empty(); });
    }

    void Disconnect()
    {
        std::scoped_lock operationLock(operationMutex_);
        bool hadClient = false;
        {
            std::scoped_lock lock(mutex_);
            shuttingDown_ = true;
            hadClient = static_cast<bool>(client_);
        }
        if (hadClient)
            SetStatus(LifecycleStatus::Disconnecting, "Disconnecting from MQTT broker");
        ReleaseClient();
        SetStatus(LifecycleStatus::Offline, "Offline");
    }

    bool Connected() const
    {
        std::scoped_lock lock(mutex_);
        return status_ == LifecycleStatus::Connected && client_ && client_->is_connected();
    }

    LifecycleStatus Lifecycle() const
    {
        std::scoped_lock lock(mutex_);
        return status_;
    }

    void SetStatusHandler(StatusHandler handler)
    {
        std::scoped_lock lock(mutex_);
        statusHandler_ = std::move(handler);
    }

    void SetErrorHandler(ErrorHandler handler)
    {
        std::scoped_lock lock(mutex_);
        errorHandler_ = std::move(handler);
    }

    void Run(std::stop_token stopToken, std::string host, int port, std::string clientId)
    {
        std::stop_callback stopCallback(stopToken, [this] { stateChanged_.notify_all(); });
        auto retryDelay = 1s;

        while (!stopToken.stop_requested()) {
            const auto state = Lifecycle();
            if (state == LifecycleStatus::Offline || state == LifecycleStatus::Error) {
                Connect(host, port, clientId);
            }

            std::unique_lock lock(mutex_);
            stateChanged_.wait_for(lock, retryDelay, [&] {
                return stopToken.stop_requested() || status_ == LifecycleStatus::Connected
                    || status_ == LifecycleStatus::Reconnecting || status_ == LifecycleStatus::Error;
            });
            if (status_ == LifecycleStatus::Error) {
                retryDelay = std::min(retryDelay * 2, 30s);
                stateChanged_.wait_for(lock, retryDelay, [&] {
                    return stopToken.stop_requested() || status_ != LifecycleStatus::Error;
                });
            }
            else {
                retryDelay = 1s;
            }

            if (status_ == LifecycleStatus::Connected || status_ == LifecycleStatus::Reconnecting) {
                stateChanged_.wait(lock, [&] {
                    return stopToken.stop_requested()
                        || (status_ != LifecycleStatus::Connected && status_ != LifecycleStatus::Reconnecting);
                });
            }
        }

        Disconnect();
    }

private:
    void connected(const std::string& cause) override
    {
        HandleConnected(cause.empty() ? "Connected to MQTT broker" : "MQTT reconnected: " + cause);
    }

    void connection_lost(const std::string& cause) override
    {
        bool shuttingDown = false;
        {
            std::scoped_lock lock(mutex_);
            shuttingDown = shuttingDown_;
        }
        if (!shuttingDown) {
            const auto detail = cause.empty() ? "MQTT connection lost; reconnecting automatically"
                                              : "MQTT connection lost: " + cause + "; reconnecting automatically";
            SetStatus(LifecycleStatus::Reconnecting, detail);
        }
    }

    void message_arrived(mqtt::const_message_ptr message) override
    {
        std::vector<MessageHandler> handlers;
        {
            std::scoped_lock lock(mutex_);
            for (const auto& [filter, handler] : subscriptions_) {
                if (TopicMatches(filter, message->get_topic()))
                    handlers.push_back(handler);
            }
        }

        if (handlers.empty()) {
            ReportError(
                std::format("Received MQTT message on {} without a registered handler", message->get_topic()), false);
            return;
        }

        for (auto& handler : handlers) {
            try {
                handler(message->get_topic(), message->get_payload_str());
            }
            catch (const std::exception& exception) {
                ReportError(
                    std::format("MQTT message handler for {} failed: {}", message->get_topic(), exception.what()), false);
            }
            catch (...) {
                ReportError(
                    std::format("MQTT message handler for {} failed with an unknown exception", message->get_topic()), false);
            }
        }
    }

    void delivery_complete(mqtt::delivery_token_ptr) override
    {
    }

    void on_failure(const mqtt::token& token) override
    {
        const auto message = token.get_error_message();
        ReportError(
            message.empty() ? std::format("MQTT connection attempt failed (code {})", token.get_return_code())
                            : "MQTT connection attempt failed: " + message,
            true);
    }

    void on_success(const mqtt::token&) override
    {
        HandleConnected("Connected to MQTT broker");
    }

    void HandleConnected(std::string detail)
    {
        std::shared_ptr<mqtt::async_client> client;
        std::vector<std::string> topics;
        StatusHandler handler;
        {
            std::scoped_lock lock(mutex_);
            if (shuttingDown_ || status_ == LifecycleStatus::Connected)
                return;
            status_ = LifecycleStatus::Connected;
            handler = statusHandler_;
            client = client_;
            topics.reserve(subscriptions_.size());
            for (const auto& [topic, messageHandler] : subscriptions_) {
                (void)messageHandler;
                topics.push_back(topic);
            }
        }
        stateChanged_.notify_all();
        if (appState_) {
            auto console = appState_->SnapshotConsole();
            console.connectionStatus = "Connected";
            appState_->UpdateConsole(console);
            appState_->AddActivity(detail);
        }
        if (handler) {
            try {
                handler(LifecycleStatus::Connected, detail);
            }
            catch (...) {
                // Never allow application callbacks to unwind into Paho threads.
            }
        }
        for (const auto& topic : topics)
            SubscribeClient(client, topic);
    }

    void SubscribeClient(const std::shared_ptr<mqtt::async_client>& client, const std::string& topic)
    {
        if (!client)
            return;
        try {
            auto listener = TrackOperation("MQTT subscription to " + topic);
            try {
                client->subscribe(topic, DeliveryQos, nullptr, *listener);
            }
            catch (...) {
                CancelOperation(*listener);
                throw;
            }
        }
        catch (const mqtt::exception& exception) {
            ReportError(std::format("MQTT subscription to {} failed: {}", topic, exception.what()), false);
        }
    }

    void ReleaseClient()
    {
        std::shared_ptr<mqtt::async_client> client;
        {
            std::scoped_lock lock(mutex_);
            client = std::exchange(client_, {});
        }
        if (!client)
            return;

        try {
            if (client->is_connected()) {
                const auto token = client->disconnect(2s);
                if (!token->wait_for(3s))
                    ReportError("Timed out waiting for MQTT disconnect", false);
            }
        }
        catch (const mqtt::exception& exception) {
            ReportError(std::format("MQTT disconnect failed: {}", exception.what()), false);
        }
        client->disable_callbacks();
        client.reset();
        {
            std::scoped_lock lock(mutex_);
            pendingOperations_.clear();
        }
        pendingOperationsChanged_.notify_all();
    }

    std::shared_ptr<OperationListener> TrackOperation(std::string operation)
    {
        auto listener = std::make_shared<OperationListener>(*this, std::move(operation));
        std::scoped_lock lock(mutex_);
        pendingOperations_.push_back(listener);
        return listener;
    }

    void CancelOperation(OperationListener& listener)
    {
        {
            std::scoped_lock lock(mutex_);
            std::erase_if(pendingOperations_, [&](const auto& pending) { return pending.get() == &listener; });
        }
        pendingOperationsChanged_.notify_all();
    }

    void CompleteOperation(OperationListener& listener, std::string error)
    {
        // Retain the listener locally while removing it from the ownership
        // collection; it must remain alive until its callback returns.
        std::shared_ptr<OperationListener> keepAlive;
        {
            std::scoped_lock lock(mutex_);
            const auto found = std::ranges::find_if(
                pendingOperations_, [&](const auto& pending) { return pending.get() == &listener; });
            if (found == pendingOperations_.end())
                return;
            keepAlive = *found;
            pendingOperations_.erase(found);
        }
        pendingOperationsChanged_.notify_all();
        if (!error.empty())
            ReportError(std::move(error), false);
    }

    void SetStatus(LifecycleStatus status, std::string detail)
    {
        StatusHandler handler;
        {
            std::scoped_lock lock(mutex_);
            status_ = status;
            handler = statusHandler_;
        }
        stateChanged_.notify_all();

        if (appState_) {
            auto console = appState_->SnapshotConsole();
            console.connectionStatus = StatusText(status);
            appState_->UpdateConsole(console);
            appState_->AddActivity(detail);
        }
        if (handler) {
            try {
                handler(status, std::move(detail));
            }
            catch (...) {
                // Never allow application callbacks to unwind into Paho threads.
            }
        }
    }

    void ReportError(std::string error, bool changesLifecycle)
    {
        ErrorHandler handler;
        bool shuttingDown = false;
        {
            std::scoped_lock lock(mutex_);
            shuttingDown = shuttingDown_;
            if (!shuttingDown && changesLifecycle)
                status_ = LifecycleStatus::Error;
            handler = errorHandler_;
        }
        stateChanged_.notify_all();

        if (appState_) {
            auto console = appState_->SnapshotConsole();
            if (!shuttingDown && changesLifecycle)
                console.connectionStatus = "Error";
            appState_->UpdateConsole(console);
            appState_->AddActivity(error);
        }
        if (handler) {
            try {
                handler(std::move(error));
            }
            catch (...) {
                // Never allow application callbacks to unwind into Paho threads.
            }
        }
    }

    AppState* appState_;
    mutable std::mutex mutex_;
    std::mutex operationMutex_;
    std::condition_variable stateChanged_;
    std::condition_variable pendingOperationsChanged_;
    std::shared_ptr<mqtt::async_client> client_;
    std::map<std::string, MessageHandler, std::less<>> subscriptions_;
    std::vector<std::shared_ptr<OperationListener>> pendingOperations_;
    StatusHandler statusHandler_;
    ErrorHandler errorHandler_;
    LifecycleStatus status_ = LifecycleStatus::Offline;
    bool shuttingDown_ = false;
};

MqttService::MqttService(AppState* appState)
    : impl_(std::make_unique<Impl>(appState))
{
}

MqttService::~MqttService() = default;

void MqttService::Connect(std::string host, int port, std::string clientId)
{
    impl_->Connect(std::move(host), port, std::move(clientId));
}

void MqttService::Subscribe(std::string topic, MessageHandler handler)
{
    impl_->Subscribe(std::move(topic), std::move(handler));
}

void MqttService::Unsubscribe(std::string topic)
{
    impl_->Unsubscribe(std::move(topic));
}

void MqttService::Publish(std::string topic, std::string payload)
{
    impl_->Publish(std::move(topic), std::move(payload));
}

bool MqttService::WaitForPendingOperations(std::chrono::milliseconds timeout)
{
    return impl_->WaitForPendingOperations(timeout);
}

void MqttService::Disconnect()
{
    impl_->Disconnect();
}

bool MqttService::Connected() const
{
    return impl_->Connected();
}

MqttService::LifecycleStatus MqttService::Lifecycle() const
{
    return impl_->Lifecycle();
}

void MqttService::SetStatusHandler(StatusHandler handler)
{
    impl_->SetStatusHandler(std::move(handler));
}

void MqttService::SetErrorHandler(ErrorHandler handler)
{
    impl_->SetErrorHandler(std::move(handler));
}

void MqttService::Run(std::stop_token stopToken, std::string host, int port, std::string clientId)
{
    impl_->Run(stopToken, std::move(host), port, std::move(clientId));
}

} // namespace Watchlist::Messaging
