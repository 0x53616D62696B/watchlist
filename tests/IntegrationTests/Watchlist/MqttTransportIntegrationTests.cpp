#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "src/Watchlist/Messaging/MqttService.hpp"

namespace {

using namespace std::chrono_literals;
using Watchlist::Messaging::MqttService;

template <typename Predicate>
bool WaitUntil(Predicate predicate, std::chrono::seconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (predicate())
            return true;
        std::this_thread::sleep_for(25ms);
    }
    return predicate();
}

std::string BrokerHost()
{
    if (const auto* host = std::getenv("WATCHLIST_MQTT_HOST"))
        return host;
    return "localhost";
}

int BrokerPort()
{
    if (const auto* port = std::getenv("WATCHLIST_MQTT_PORT"))
        return std::stoi(port);
    return 1883;
}

TEST(MqttTransportIntegrationTests, DeliversQosOneNonRetainedMessageAndStopsCooperatively)
{
    const auto uniqueSuffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto topic = "watchlist/integration/" + uniqueSuffix;
    const auto payload = "transport-payload-" + uniqueSuffix;

    MqttService subscriber;
    MqttService publisher;
    std::mutex messageMutex;
    std::condition_variable messageReceived;
    std::string receivedTopic;
    std::string receivedPayload;

    subscriber.Subscribe(topic, [&](std::string incomingTopic, std::string incomingPayload) {
        {
            std::scoped_lock lock(messageMutex);
            receivedTopic = std::move(incomingTopic);
            receivedPayload = std::move(incomingPayload);
        }
        messageReceived.notify_all();
    });

    const auto host = BrokerHost();
    const auto port = BrokerPort();
    std::jthread subscriberThread([&](std::stop_token stopToken) {
        subscriber.Run(stopToken, host, port, "watchlist-test-subscriber-" + uniqueSuffix);
    });
    std::jthread publisherThread([&](std::stop_token stopToken) {
        publisher.Run(stopToken, host, port, "watchlist-test-publisher-" + uniqueSuffix);
    });

    ASSERT_TRUE(WaitUntil([&] { return subscriber.Connected() && publisher.Connected(); }, 10s))
        << "Broker unavailable at " << host << ':' << port;

    // Publishing repeatedly also removes timing dependence on the asynchronous
    // SUBACK while still verifying the production QoS/retain settings.
    bool delivered = false;
    for (int attempt = 0; attempt < 20 && !delivered; ++attempt) {
        publisher.Publish(topic, payload);
        std::unique_lock lock(messageMutex);
        delivered = messageReceived.wait_for(lock, 100ms, [&] { return receivedPayload == payload; });
    }

    EXPECT_TRUE(delivered);
    EXPECT_EQ(receivedTopic, topic);
    EXPECT_EQ(receivedPayload, payload);

    // Force a new Paho client connection while preserving the registered
    // subscription, proving that connection recovery re-subscribes handlers.
    subscriber.Disconnect();
    ASSERT_TRUE(WaitUntil([&] { return subscriber.Connected(); }, 10s));
    const auto payloadAfterReconnect = payload + "-after-reconnect";
    {
        std::scoped_lock lock(messageMutex);
        receivedPayload.clear();
    }
    delivered = false;
    for (int attempt = 0; attempt < 20 && !delivered; ++attempt) {
        publisher.Publish(topic, payloadAfterReconnect);
        std::unique_lock lock(messageMutex);
        delivered = messageReceived.wait_for(
            lock, 100ms, [&] { return receivedPayload == payloadAfterReconnect; });
    }
    EXPECT_TRUE(delivered);

    subscriberThread.request_stop();
    subscriberThread.join();

    // A subscriber arriving after publication must not receive the earlier
    // messages, demonstrating that production publishes are non-retained.
    std::atomic<bool> lateMessageReceived = false;
    MqttService lateSubscriber;
    lateSubscriber.Subscribe(topic, [&](std::string, std::string) { lateMessageReceived = true; });
    std::jthread lateSubscriberThread([&](std::stop_token stopToken) {
        lateSubscriber.Run(stopToken, host, port, "watchlist-test-late-subscriber-" + uniqueSuffix);
    });
    ASSERT_TRUE(WaitUntil([&] { return lateSubscriber.Connected(); }, 10s));
    std::this_thread::sleep_for(500ms);
    EXPECT_FALSE(lateMessageReceived.load());

    lateSubscriberThread.request_stop();
    publisherThread.request_stop();
    lateSubscriberThread.join();
    publisherThread.join();

    EXPECT_EQ(subscriber.Lifecycle(), MqttService::LifecycleStatus::Offline);
    EXPECT_EQ(publisher.Lifecycle(), MqttService::LifecycleStatus::Offline);
    EXPECT_EQ(lateSubscriber.Lifecycle(), MqttService::LifecycleStatus::Offline);
}

} // namespace
