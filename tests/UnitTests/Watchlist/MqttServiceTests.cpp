#include <chrono>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "src/Watchlist/Messaging/MqttService.hpp"

namespace {

using Watchlist::Messaging::MqttService;

TEST(MqttServiceTests, InvalidConnectionConfigurationIsReportedWithoutBrokerAccess)
{
    MqttService service;
    std::vector<std::string> errors;
    service.SetErrorHandler([&](std::string error) { errors.push_back(std::move(error)); });

    service.Connect("", 1883, "client");

    EXPECT_EQ(service.Lifecycle(), MqttService::LifecycleStatus::Error);
    ASSERT_EQ(errors.size(), 1);
    EXPECT_NE(errors.front().find("host"), std::string::npos);
    EXPECT_FALSE(service.Connected());
}

TEST(MqttServiceTests, MisuseReportsAnErrorWithoutChangingOfflineLifecycle)
{
    MqttService service;
    std::vector<std::string> errors;
    service.SetErrorHandler([&](std::string error) { errors.push_back(std::move(error)); });

    service.Publish("watchlist/test", "payload");

    EXPECT_EQ(service.Lifecycle(), MqttService::LifecycleStatus::Offline);
    ASSERT_EQ(errors.size(), 1);
    EXPECT_NE(errors.front().find("before Connect"), std::string::npos);
}

TEST(MqttServiceTests, DisconnectIsIdempotentWithoutBrokerAccess)
{
    MqttService service;

    service.Disconnect();
    service.Disconnect();

    EXPECT_EQ(service.Lifecycle(), MqttService::LifecycleStatus::Offline);
    EXPECT_FALSE(service.Connected());
}

TEST(MqttServiceTests, OfflineUnsubscribeAndPendingDrainAreImmediate)
{
    MqttService service;

    service.Subscribe("watchlist/requests", [](std::string, std::string) {});
    service.Unsubscribe("watchlist/requests");

    EXPECT_TRUE(service.WaitForPendingOperations(std::chrono::milliseconds(10)));
}

} // namespace
