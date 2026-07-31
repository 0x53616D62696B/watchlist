#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "src/Watchlist/AppState.hpp"
#include "src/Watchlist/Messaging/MqttMessages.hpp"
#include "src/Watchlist/Messaging/MqttService.hpp"

namespace {

TEST(AppStateTests, DispatchesSerializedRequestAndRecordsOutboundPayload)
{
    Watchlist::AppState state;
    std::string publishedPayload;
    state.SetOutboundCommandHandler([&](std::string payload) { publishedPayload = std::move(payload); });
    const Watchlist::Messaging::MqttRequest request {
        .id = "request-42",
        .type = Watchlist::Messaging::MessageType::CmdToDevice,
        .createdUtc = "2026-07-31T12:00:00Z",
        .sourceClientId = "client-7",
        .deviceId = "device-3",
        .command = "restart",
    };

    EXPECT_TRUE(state.DispatchOutbound(Watchlist::Messaging::SerializeRequest(request)));

    const auto parsed = Watchlist::Messaging::ParseRequest(publishedPayload);
    ASSERT_TRUE(parsed.request.has_value());
    EXPECT_EQ(parsed.request->id, request.id);
    EXPECT_EQ(parsed.request->sourceClientId, request.sourceClientId);
    EXPECT_EQ(parsed.request->command, request.command);
    ASSERT_EQ(state.Outbound().size(), 1);
    EXPECT_EQ(state.Outbound().front(), publishedPayload);
}

TEST(AppStateTests, MissingCommandHandlerRecordsFailure)
{
    Watchlist::AppState state;

    EXPECT_FALSE(state.DispatchOutbound("payload"));

    EXPECT_TRUE(state.Outbound().empty());
    ASSERT_EQ(state.Activity().size(), 1);
    EXPECT_NE(state.Activity().front().find("no outbound command handler"), std::string::npos);
}

TEST(AppStateTests, ThrowingCommandHandlerRecordsFailureWithoutOutboundMessage)
{
    Watchlist::AppState state;
    state.SetOutboundCommandHandler([](std::string) { throw std::runtime_error("publish rejected"); });

    EXPECT_FALSE(state.DispatchOutbound("payload"));

    EXPECT_TRUE(state.Outbound().empty());
    ASSERT_EQ(state.Activity().size(), 1);
    EXPECT_NE(state.Activity().front().find("publish rejected"), std::string::npos);
}

TEST(AppStateTests, CommandHandlerCanReenterAppStateWithoutDeadlock)
{
    Watchlist::AppState state;
    state.SetOutboundCommandHandler([&](std::string) { state.AddActivity("callback completed"); });

    EXPECT_TRUE(state.DispatchOutbound("payload"));

    ASSERT_EQ(state.Activity().size(), 1);
    EXPECT_EQ(state.Activity().front(), "callback completed");
}

TEST(AppStateTests, TransportFailureUpdatesConnectionStatusAndActivity)
{
    Watchlist::AppState state;
    Watchlist::Messaging::MqttService service(&state);

    service.Connect("", 1883, "client");

    EXPECT_EQ(state.SnapshotConsole().connectionStatus, "Error");
    ASSERT_EQ(state.Activity().size(), 1);
    EXPECT_NE(state.Activity().front().find("host"), std::string::npos);
}

} // namespace
