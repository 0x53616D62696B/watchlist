#include <chrono>
#include <filesystem>
#include <future>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Watchlist/Dispatch/RequestDispatcher.hpp"
#include "src/Watchlist/Messaging/MqttMessages.hpp"
#include "src/Watchlist/Storage/KeyValueStore.hpp"

namespace {

using Watchlist::Messaging::MessageType;
using Watchlist::Messaging::MqttRequest;

MqttRequest BaseRequest(MessageType type)
{
    return {
        .id = "request-1",
        .type = type,
        .createdUtc = "2026-06-13T12:00:00Z",
        .sourceClientId = "client-1",
    };
}

MqttRequest RequestFor(MessageType type)
{
    auto request = BaseRequest(type);

    switch (type) {
    case MessageType::ExecuteScript:
        request.scriptName = "script1";
        break;
    case MessageType::SendToDevice:
        request.deviceId = "device-1";
        request.payload = "payload";
        break;
    case MessageType::CmdToDevice:
        request.deviceId = "device-1";
        request.command = "reboot";
        break;
    case MessageType::StoreDatabaseValue:
        request.key = "temperature";
        request.value = "21.5";
        break;
    case MessageType::QueryAiPrompt:
        request.prompt = "Summarize the log";
        break;
    }

    return request;
}

std::filesystem::path TempDatabasePath(std::string_view name)
{
    const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (std::string(name) + std::to_string(suffix) + ".sqlite");
}

} // namespace

TEST(MqttMessagingTests, RoundTripsAllRequestTypes)
{
    for (const auto type : {
             MessageType::ExecuteScript,
             MessageType::SendToDevice,
             MessageType::CmdToDevice,
             MessageType::StoreDatabaseValue,
             MessageType::QueryAiPrompt,
         }) {
        const auto original = RequestFor(type);
        const auto parsed = Watchlist::Messaging::ParseRequest(Watchlist::Messaging::SerializeRequest(original));

        ASSERT_TRUE(parsed.request.has_value());
        EXPECT_EQ(parsed.request->id, original.id);
        EXPECT_EQ(parsed.request->type, original.type);
        EXPECT_EQ(parsed.request->createdUtc, original.createdUtc);
        EXPECT_EQ(parsed.request->sourceClientId, original.sourceClientId);
    }
}

TEST(MqttMessagingTests, InvalidPayloadReturnsDeterministicErrors)
{
    EXPECT_EQ(Watchlist::Messaging::ParseRequest("not JSON").error, "invalid JSON object");
    EXPECT_EQ(Watchlist::Messaging::ParseRequest("{}").error, "missing required field: type");
    EXPECT_EQ(
        Watchlist::Messaging::ParseRequest(R"({"type":"not_real"})").error,
        "unknown message type: not_real");
    EXPECT_EQ(
        Watchlist::Messaging::ParseRequest(R"({"type":"execute_script","id":"1","created_utc":"now","source_client_id":"client"})").error,
        "missing required field: script_name");
}

TEST(MqttMessagingTests, CallbackQueuesValidWorkWithoutWaitingForWorker)
{
    Concurrency::ThreadPoolManager runtime(1);
    std::promise<void> blockerStarted;
    std::promise<void> releaseBlocker;
    auto releaseFuture = releaseBlocker.get_future().share();
    auto blocker = runtime.EnqueueTask([&] {
        blockerStarted.set_value();
        releaseFuture.wait();
    });
    ASSERT_EQ(blockerStarted.get_future().wait_for(std::chrono::seconds(1)), std::future_status::ready);

    std::promise<Watchlist::Messaging::MqttAck> published;
    auto publishedFuture = published.get_future();
    Watchlist::Dispatch::RequestDispatcher dispatcher(
        runtime,
        TempDatabasePath("async_dispatcher_"),
        nullptr,
        [&](const Watchlist::Messaging::MqttRequest&, const Watchlist::Messaging::MqttAck& ack) {
            published.set_value(ack);
        });

    EXPECT_TRUE(dispatcher.HandleIncomingPayload(
        Watchlist::Messaging::SerializeRequest(RequestFor(MessageType::ExecuteScript))));
    EXPECT_EQ(publishedFuture.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);

    releaseBlocker.set_value();
    blocker.get();
    ASSERT_EQ(publishedFuture.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    EXPECT_EQ(publishedFuture.get().status, "ok");
}

TEST(MqttMessagingTests, ValidationErrorPublishesOnlyWithSafelyRecoveredClient)
{
    Concurrency::ThreadPoolManager runtime(1);
    std::vector<std::pair<Watchlist::Messaging::MqttRequest, Watchlist::Messaging::MqttAck>> published;
    Watchlist::Dispatch::RequestDispatcher dispatcher(
        runtime,
        TempDatabasePath("validation_dispatcher_"),
        nullptr,
        [&](const Watchlist::Messaging::MqttRequest& request, const Watchlist::Messaging::MqttAck& ack) {
            published.emplace_back(request, ack);
        });

    EXPECT_FALSE(dispatcher.HandleIncomingPayload(
        R"({"type":"execute_script","id":"bad-1","created_utc":"now","source_client_id":"client-1"})"));
    ASSERT_EQ(published.size(), 1);
    EXPECT_EQ(published.front().first.sourceClientId, "client-1");
    EXPECT_EQ(published.front().second.requestId, "bad-1");
    EXPECT_EQ(published.front().second.message, "missing required field: script_name");

    EXPECT_FALSE(dispatcher.HandleIncomingPayload(
        R"({"type":"execute_script","id":"bad-2","source_client_id":"client-1",)"));
    EXPECT_EQ(published.size(), 1);

    EXPECT_FALSE(dispatcher.HandleIncomingPayload(
        R"({"type":"execute_script","id":"bad-3","created_utc":"now","source_client_id":"bad/topic"})"));
    EXPECT_EQ(published.size(), 1);
}

TEST(MqttMessagingTests, ShutdownRejectsNewRequestsAndDrainsQueuedAcknowledgement)
{
    Concurrency::ThreadPoolManager runtime(1);
    std::promise<void> blockerStarted;
    std::promise<void> releaseBlocker;
    auto releaseFuture = releaseBlocker.get_future().share();
    auto blocker = runtime.EnqueueTask([&] {
        blockerStarted.set_value();
        releaseFuture.wait();
    });
    ASSERT_EQ(blockerStarted.get_future().wait_for(std::chrono::seconds(1)), std::future_status::ready);

    std::vector<Watchlist::Messaging::MqttAck> published;
    Watchlist::Dispatch::RequestDispatcher dispatcher(
        runtime,
        TempDatabasePath("shutdown_dispatcher_"),
        nullptr,
        [&](const Watchlist::Messaging::MqttRequest&, const Watchlist::Messaging::MqttAck& ack) {
            published.push_back(ack);
        });

    EXPECT_TRUE(dispatcher.HandleIncomingPayload(
        Watchlist::Messaging::SerializeRequest(RequestFor(MessageType::ExecuteScript))));
    dispatcher.StopAccepting();
    EXPECT_FALSE(dispatcher.HandleIncomingPayload(
        Watchlist::Messaging::SerializeRequest(RequestFor(MessageType::ExecuteScript))));

    runtime.StopAll();
    releaseBlocker.set_value();
    runtime.JoinAll();
    blocker.get();

    ASSERT_EQ(published.size(), 2);
    EXPECT_EQ(published[0].status, "error");
    EXPECT_EQ(published[0].message, "server shutting down");
    EXPECT_EQ(published[1].status, "ok");
}

TEST(MqttMessagingTests, DispatcherRoutesWorkerRequestsAndPublishesAck)
{
    Concurrency::ThreadPoolManager runtime(1);
    const auto databasePath = TempDatabasePath("dispatcher_");
    std::string publishedTopicClient;
    std::string publishedStatus;

    Watchlist::Dispatch::RequestDispatcher dispatcher(
        runtime,
        databasePath,
        nullptr,
        [&](const Watchlist::Messaging::MqttRequest& request, const Watchlist::Messaging::MqttAck& ack) {
            publishedTopicClient = request.sourceClientId;
            publishedStatus = ack.status;
        });

    auto future = dispatcher.Dispatch(RequestFor(MessageType::StoreDatabaseValue));
    const auto ack = future.get();

    EXPECT_EQ(ack.status, "ok");
    EXPECT_EQ(publishedTopicClient, "client-1");
    EXPECT_EQ(publishedStatus, "ok");

    {
        Watchlist::Storage::KeyValueStore store(databasePath);
        store.Initialize();
        EXPECT_EQ(store.Get("temperature"), "21.5");
    }

    std::filesystem::remove(databasePath);
}

TEST(MqttMessagingTests, RuntimeStartsStopsJoinsAndRejectsAfterStop)
{
    Concurrency::ThreadPoolManager runtime(1);
    std::promise<void> started;
    auto startedFuture = started.get_future();

    runtime.StartDedicatedThread("test-service", [&started](std::stop_token stopToken) mutable {
        started.set_value();
        while (!stopToken.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    ASSERT_EQ(startedFuture.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    runtime.StopAll();
    runtime.JoinAll();

    EXPECT_THROW(runtime.EnqueueTask([] {}), std::runtime_error);
}

TEST(MqttMessagingTests, KeyValueStoreUpsertsAndGetsValues)
{
    const auto databasePath = TempDatabasePath("kv_store_");
    {
        Watchlist::Storage::KeyValueStore store(databasePath);

        store.Initialize();
        store.Upsert("mode", "initial");
        store.Upsert("mode", "updated");

        EXPECT_EQ(store.Get("mode"), "updated");
        EXPECT_FALSE(store.Get("missing").has_value());
    }

    std::filesystem::remove(databasePath);
}
