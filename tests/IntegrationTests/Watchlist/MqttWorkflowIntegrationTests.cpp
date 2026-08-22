#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <gtest/gtest.h>

#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Watchlist/AppState.hpp"
#include "src/Watchlist/Dispatch/RequestDispatcher.hpp"
#include "src/Watchlist/Messaging/MqttMessages.hpp"
#include "src/Watchlist/Messaging/MqttService.hpp"
#include "src/Watchlist/Messaging/MqttTopics.hpp"
#include "src/Watchlist/Storage/KeyValueStore.hpp"

namespace {

using namespace std::chrono_literals;
using Watchlist::Messaging::MqttService;

std::string EnvironmentOr(std::string_view name, std::string fallback)
{
    if (const auto* value = std::getenv(std::string(name).c_str()))
        return value;
    return fallback;
}

int BrokerPort()
{
    return std::stoi(EnvironmentOr("WATCHLIST_MQTT_PORT", "1883"));
}

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

bool ContainsJsonString(const std::string& json, std::string_view field, std::string_view value)
{
    return json.find(std::format(R"("{}":"{}")", field, value)) != std::string::npos;
}

int RestartManagedBroker()
{
    const auto cmake = EnvironmentOr("WATCHLIST_CMAKE_COMMAND", "cmake");
    const auto fixture = EnvironmentOr("WATCHLIST_MQTT_FIXTURE_SCRIPT", "");
    const auto compose = EnvironmentOr("WATCHLIST_MQTT_COMPOSE_FILE", "");
    const auto project = EnvironmentOr("WATCHLIST_MQTT_COMPOSE_PROJECT", "");
    const auto port = EnvironmentOr("WATCHLIST_MQTT_PORT", "1883");
    std::vector<std::string> arguments {
        cmake,
        "-DACTION=restart",
        "-DCOMPOSE_FILE=" + compose,
        "-DCOMPOSE_PROJECT=" + project,
        "-DBROKER_PORT=" + port,
        "-P",
        fixture,
    };
#ifdef _WIN32
    std::vector<const char*> argumentPointers;
    argumentPointers.reserve(arguments.size() + 1);
    for (const auto& argument : arguments)
        argumentPointers.push_back(argument.c_str());
    argumentPointers.push_back(nullptr);
    return static_cast<int>(::_spawnv(_P_WAIT, cmake.c_str(), argumentPointers.data()));
#else
    std::vector<char*> argumentPointers;
    argumentPointers.reserve(arguments.size() + 1);
    for (auto& argument : arguments)
        argumentPointers.push_back(argument.data());
    argumentPointers.push_back(nullptr);
    const auto process = ::fork();
    if (process == 0) {
        ::execv(cmake.c_str(), argumentPointers.data());
        ::_exit(127);
    }
    if (process < 0)
        return -1;
    int status = 0;
    if (::waitpid(process, &status, 0) < 0)
        return -1;
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
}

TEST(MqttWorkflowIntegrationTests, PublishesDispatchesAndReturnsSuccessAndValidationAcks)
{
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto clientId = "watchlist-workflow-client-" + suffix;
    const auto databasePath = std::filesystem::temp_directory_path() / ("watchlist_mqtt_" + suffix + ".sqlite");
    const auto host = EnvironmentOr("WATCHLIST_MQTT_HOST", "localhost");
    const auto port = BrokerPort();

    Concurrency::ThreadPoolManager workers(2);
    Watchlist::AppState serverState;
    Watchlist::AppState clientState;
    MqttService serverTransport(&serverState);
    MqttService clientTransport(&clientState);
    Watchlist::Dispatch::RequestDispatcher dispatcher(
        workers,
        databasePath,
        &serverState,
        [&](const Watchlist::Messaging::MqttRequest& request, const Watchlist::Messaging::MqttAck& ack) {
            serverTransport.Publish(
                Watchlist::Messaging::AckTopicForClient(request.sourceClientId),
                Watchlist::Messaging::SerializeAck(ack));
        });

    std::mutex ackMutex;
    std::condition_variable ackReceived;
    std::vector<std::string> clientAcks;
    clientTransport.Subscribe(
        Watchlist::Messaging::AckTopicForClient(clientId),
        [&](std::string, std::string payload) {
            {
                std::scoped_lock lock(ackMutex);
                clientAcks.push_back(std::move(payload));
            }
            ackReceived.notify_all();
        });
    serverTransport.Subscribe(Watchlist::Messaging::RequestsTopic, [&](std::string, std::string payload) {
        [[maybe_unused]] const bool accepted = dispatcher.HandleIncomingPayload(payload);
    });

    std::jthread serverThread([&](std::stop_token token) {
        serverTransport.Run(token, host, port, "watchlist-workflow-server-" + suffix);
    });
    std::jthread clientThread([&](std::stop_token token) {
        clientTransport.Run(token, host, port, clientId);
    });
    ASSERT_TRUE(WaitUntil([&] { return serverTransport.Connected() && clientTransport.Connected(); }, 15s));

    const Watchlist::Messaging::MqttRequest request {
        .id = "store-" + suffix,
        .type = Watchlist::Messaging::MessageType::StoreDatabaseValue,
        .createdUtc = Watchlist::Messaging::UtcNowIso8601(),
        .sourceClientId = clientId,
        .key = "integration-key",
        .value = "integration-value",
    };
    const auto requestPayload = Watchlist::Messaging::SerializeRequest(request);
    bool successAckReceived = false;
    for (int attempt = 0; attempt < 30 && !successAckReceived; ++attempt) {
        clientTransport.Publish(Watchlist::Messaging::RequestsTopic, requestPayload);
        std::unique_lock lock(ackMutex);
        successAckReceived = ackReceived.wait_for(lock, 100ms, [&] {
            return std::ranges::any_of(clientAcks, [&](const auto& ack) {
                return ContainsJsonString(ack, "request_id", request.id)
                    && ContainsJsonString(ack, "status", "ok");
            });
        });
    }
    ASSERT_TRUE(successAckReceived);
    ASSERT_FALSE(serverState.Received().empty());
    EXPECT_EQ(serverState.Received().front(), requestPayload);
    EXPECT_TRUE(std::ranges::any_of(serverState.Activity(), [&](const auto& activity) {
        return activity.find("Queued store_database_value request " + request.id) != std::string::npos;
    }));
    {
        Watchlist::Storage::KeyValueStore store(databasePath);
        store.Initialize();
        EXPECT_EQ(store.Get("integration-key"), "integration-value");
    }

    const auto malformedId = "malformed-" + suffix;
    const auto malformed = std::format(
        R"({{"id":"{}","type":"execute_script","created_utc":"now","source_client_id":"{}"}})",
        malformedId,
        clientId);
    bool errorAckReceived = false;
    for (int attempt = 0; attempt < 30 && !errorAckReceived; ++attempt) {
        clientTransport.Publish(Watchlist::Messaging::RequestsTopic, malformed);
        std::unique_lock lock(ackMutex);
        errorAckReceived = ackReceived.wait_for(lock, 100ms, [&] {
            return std::ranges::any_of(clientAcks, [&](const auto& ack) {
                return ContainsJsonString(ack, "request_id", malformedId)
                    && ContainsJsonString(ack, "status", "error")
                    && ContainsJsonString(ack, "message", "missing required field: script_name");
            });
        });
    }
    EXPECT_TRUE(errorAckReceived);

    dispatcher.StopAccepting();
    ASSERT_TRUE(serverTransport.WaitForPendingOperations(3s));
    serverThread.request_stop();
    clientThread.request_stop();
    serverThread.join();
    clientThread.join();
    std::filesystem::remove(databasePath);
}

TEST(MqttWorkflowIntegrationTests, AutomaticallyReconnectsAndRestoresSubscriptionAfterBrokerRestart)
{
    const auto* fixtureScript = std::getenv("WATCHLIST_MQTT_FIXTURE_SCRIPT");
    if (!fixtureScript)
        GTEST_SKIP() << "Reconnect test requires the Docker-managed CTest broker fixture";

    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto topic = "watchlist/integration/reconnect/" + suffix;
    const auto host = EnvironmentOr("WATCHLIST_MQTT_HOST", "localhost");
    const auto port = BrokerPort();
    MqttService subscriber;
    MqttService publisher;
    std::atomic<bool> reconnectObserved = false;
    std::atomic<bool> received = false;
    subscriber.SetStatusHandler([&](MqttService::LifecycleStatus status, std::string) {
        if (status == MqttService::LifecycleStatus::Reconnecting)
            reconnectObserved = true;
    });
    subscriber.Subscribe(topic, [&](std::string, std::string payload) {
        if (payload == "after-restart")
            received = true;
    });

    std::jthread subscriberThread([&](std::stop_token token) {
        subscriber.Run(token, host, port, "watchlist-reconnect-subscriber-" + suffix);
    });
    std::jthread publisherThread([&](std::stop_token token) {
        publisher.Run(token, host, port, "watchlist-reconnect-publisher-" + suffix);
    });
    ASSERT_TRUE(WaitUntil([&] { return subscriber.Connected() && publisher.Connected(); }, 15s));

    ASSERT_EQ(RestartManagedBroker(), 0);
    ASSERT_TRUE(WaitUntil([&] { return reconnectObserved.load(); }, 30s));
    ASSERT_TRUE(WaitUntil([&] { return subscriber.Connected() && publisher.Connected(); }, 45s));

    for (int attempt = 0; attempt < 30 && !received.load(); ++attempt) {
        publisher.Publish(topic, "after-restart");
        std::this_thread::sleep_for(100ms);
    }
    EXPECT_TRUE(received.load());

    subscriberThread.request_stop();
    publisherThread.request_stop();
    subscriberThread.join();
    publisherThread.join();
}

} // namespace
