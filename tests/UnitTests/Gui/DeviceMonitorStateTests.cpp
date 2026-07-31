#include "src/Gui/DeviceMonitorState.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace Watchlist::Gui {
namespace {

DeviceViewModel Device(std::string id, bool alive = false)
{
    return {.id = std::move(id), .name = "Router", .address = "router.local", .port = 443, .alive = alive};
}

TEST(DeviceMonitorStateTests, EmitsRefreshAndPersistsExitRequest)
{
    DeviceMonitorState state;
    state.RequestRefresh();
    state.RequestExit();

    ASSERT_TRUE(state.ExitRequested());
    const auto commands = state.ConsumeCommands();
    ASSERT_EQ(commands.size(), 1U);
    EXPECT_EQ(commands.front().kind, DeviceCommandKind::Refresh);
    EXPECT_TRUE(state.ConsumeCommands().empty());
    EXPECT_TRUE(state.ExitRequested());
}

TEST(DeviceMonitorStateTests, AddsDeviceAndEmitsCompleteCommand)
{
    DeviceMonitorState state;
    state.BeginAdd();
    ASSERT_TRUE(state.Editor().has_value());
    auto& draft = state.EditSession()->draft;
    draft = {.id = "gateway", .name = "Gateway", .address = "192.0.2.1", .port = 65535, .alive = true};

    ASSERT_TRUE(state.SubmitEditor());
    ASSERT_EQ(state.Devices().size(), 1U);
    const auto commands = state.ConsumeCommands();
    ASSERT_EQ(commands.size(), 1U);
    EXPECT_EQ(commands.front().kind, DeviceCommandKind::Add);
    ASSERT_TRUE(commands.front().device.has_value());
    EXPECT_EQ(*commands.front().device, state.Devices().front());
}

TEST(DeviceMonitorStateTests, ExistingIdIsImmutable)
{
    DeviceMonitorState state;
    state.ReplaceDevices({Device("original")});
    ASSERT_TRUE(state.BeginEdit("original"));
    auto& draft = state.EditSession()->draft;
    draft.id = "changed";

    EXPECT_FALSE(state.SubmitEditor());
    ASSERT_TRUE(state.Error().has_value());
    EXPECT_EQ(state.Devices().front().id, "original");
    EXPECT_TRUE(state.ConsumeCommands().empty());
}

TEST(DeviceMonitorStateTests, EditsMutableFieldsAndAliveStatus)
{
    DeviceMonitorState state;
    state.ReplaceDevices({Device("sensor")});
    ASSERT_TRUE(state.BeginEdit("sensor"));
    auto& draft = state.EditSession()->draft;
    draft.name = "Temperature sensor";
    draft.address = "2001:db8::1";
    draft.port = 1234;
    ASSERT_TRUE(state.SubmitEditor());
    ASSERT_TRUE(state.SetAlive("sensor", true));

    EXPECT_EQ(state.Devices().front().name, "Temperature sensor");
    EXPECT_TRUE(state.Devices().front().alive);
    const auto commands = state.ConsumeCommands();
    ASSERT_EQ(commands.size(), 2U);
    EXPECT_EQ(commands[0].kind, DeviceCommandKind::Edit);
    EXPECT_EQ(commands[1].kind, DeviceCommandKind::SetAlive);
}

TEST(DeviceMonitorStateTests, DeleteRequiresExplicitConfirmation)
{
    DeviceMonitorState state;
    state.ReplaceDevices({Device("one"), Device("two")});

    ASSERT_TRUE(state.RequestDelete("one"));
    EXPECT_EQ(state.Devices().size(), 2U);
    EXPECT_TRUE(state.ConsumeCommands().empty());
    state.CancelDelete();
    EXPECT_EQ(state.Devices().size(), 2U);

    ASSERT_TRUE(state.RequestDelete("one"));
    ASSERT_TRUE(state.ConfirmDelete());
    ASSERT_EQ(state.Devices().size(), 1U);
    EXPECT_EQ(state.Devices().front().id, "two");
    const auto commands = state.ConsumeCommands();
    ASSERT_EQ(commands.size(), 1U);
    EXPECT_EQ(commands.front().kind, DeviceCommandKind::Delete);
    EXPECT_EQ(commands.front().targetId, "one");
}

TEST(DeviceMonitorStateTests, InvalidDraftLeavesEditorOpenWithError)
{
    DeviceMonitorState state;
    state.BeginAdd();
    auto& draft = state.EditSession()->draft;
    draft = {.id = "bad", .name = "Bad", .address = "host", .port = 65536};

    EXPECT_FALSE(state.SubmitEditor());
    EXPECT_TRUE(state.Editor().has_value());
    EXPECT_TRUE(state.Error().has_value());
    EXPECT_TRUE(state.Devices().empty());
}

} // namespace
} // namespace Watchlist::Gui
