#include "DetachableAppWorkspace.hpp"

#include "DetachableApp.hpp"
#include "src/Gui/Apps/AiChatApp.hpp"
#include "src/Gui/Apps/HomeAssistantApp.hpp"
#include "src/Gui/Apps/WatchlistApp.hpp"
#include "src/Gui/DeviceMonitorState.hpp"

#include <imgui.h>

#include <array>
#include <cstddef>

namespace
{
constexpr std::array Applications{
    Watchlist::Gui::DetachableAppDescriptor{
        Watchlist::Gui::DetachableAppId::Watchlist, "Watchlist", "WL", "Devices and MQTT requests",
        "Watchlist###DetachedWatchlist", &Watchlist::Gui::ShowWatchlistAppContent},
    Watchlist::Gui::DetachableAppDescriptor{
        Watchlist::Gui::DetachableAppId::HomeAssistant, "Home Assistant", "HA", "Rooms, devices, and automations",
        "Home Assistant###DetachedHomeAssistant", &Watchlist::Gui::ShowHomeAssistantAppContent},
    Watchlist::Gui::DetachableAppDescriptor{
        Watchlist::Gui::DetachableAppId::AiChat, "AI Chat", "AI", "Conversations and model controls",
        "AI Chat###DetachedAiChat", &Watchlist::Gui::ShowAiChatAppContent},
};

struct WorkspaceState
{
    std::size_t activeApplication = 0;
    std::array<bool, Applications.size()> detached{};
    bool showApplicationMap = false;
};

void ShowApplicationRail(WorkspaceState& state, Watchlist::Gui::DetachableAppContext const& context,
    bool* showConsole)
{
    ImGuiWindowFlags const flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    if (!ImGui::Begin(Watchlist::Gui::ApplicationRailWindowName, nullptr, flags))
    {
        ImGui::End();
        return;
    }

    ImGui::TextDisabled("WORKSPACE");
    ImGui::TextWrapped("Choose an app or pop it into its own ImGui window.");
    ImGui::Spacing();
    ImGui::Separator();

    for (std::size_t index = 0; index < Applications.size(); ++index)
    {
        auto const& application = Applications[index];
        ImGui::PushID(static_cast<int>(index));

        bool const selected = state.activeApplication == index;
        if (ImGui::Selectable(application.name, selected, ImGuiSelectableFlags_None, ImVec2(0.0f, 30.0f)))
            state.activeApplication = index;

        ImGui::TextDisabled("%s  %s", application.shortName, application.description);
        if (ImGui::SmallButton(state.detached[index] ? "Attach to center" : "Open separately"))
        {
            state.activeApplication = index;
            state.detached[index] = !state.detached[index];
        }
        ImGui::Spacing();
        ImGui::PopID();
    }

    ImGui::Separator();
    ImGui::Checkbox("Console (F12)", showConsole);
    if (ImGui::Button("Application map", ImVec2(-1.0f, 0.0f)))
        state.showApplicationMap = true;
    if (ImGui::Button("Exit", ImVec2(-1.0f, 0.0f)))
        context.deviceMonitorState.RequestExit();

    ImGui::End();
}

void ShowCentralApplication(
    WorkspaceState& state, Watchlist::Gui::DetachableAppContext const& context)
{
    if (!ImGui::Begin(
            Watchlist::Gui::ApplicationContentWindowName, nullptr, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    auto const& application = Applications[state.activeApplication];
    ImGui::TextDisabled("ACTIVE APP");
    ImGui::SameLine();
    ImGui::TextUnformatted(application.name);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 110.0f);

    if (state.detached[state.activeApplication])
    {
        if (ImGui::Button("Attach here"))
            state.detached[state.activeApplication] = false;
    }
    else if (ImGui::Button("Detach app"))
    {
        state.detached[state.activeApplication] = true;
    }

    ImGui::Separator();
    ImGui::Spacing();

    if (state.detached[state.activeApplication])
    {
        ImGui::TextWrapped("%s is running in a separate ImGui window.", application.name);
        ImGui::TextDisabled("The center remains the switchable workspace for every attached app.");
    }
    else
    {
        application.render(context);
    }

    ImGui::End();
}

void ShowDetachedApplications(
    WorkspaceState& state, Watchlist::Gui::DetachableAppContext const& context)
{
    for (std::size_t index = 0; index < Applications.size(); ++index)
    {
        if (!state.detached[index])
            continue;

        auto const& application = Applications[index];
        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(640.0f, 480.0f), ImGuiCond_FirstUseEver);
        bool const visible = ImGui::Begin(application.detachedWindowName, &open);
        if (visible)
        {
            ImGui::TextDisabled("DETACHED APP  |  shared shell, isolated frontend");
            ImGui::Separator();
            application.render(context);
        }
        ImGui::End();

        if (!open)
            state.detached[index] = false;
    }
}

void ShowApplicationMap(WorkspaceState& state)
{
    if (!state.showApplicationMap)
        return;

    ImGui::SetNextWindowSize(ImVec2(720.0f, 390.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Application map###ApplicationMap", &state.showApplicationMap))
    {
        ImGui::End();
        return;
    }

    ImGui::TextWrapped(
        "Apps are independent by default. Shared framework services are the deliberate common boundary; direct "
        "app-to-app links should be added here when they become necessary.");
    ImGui::Spacing();

    ImGuiTableFlags const flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
    if (ImGui::BeginTable("ApplicationRelationships", 3, flags))
    {
        ImGui::TableSetupColumn("Detachable app");
        ImGui::TableSetupColumn("Shared framework");
        ImGui::TableSetupColumn("Direct app links");
        ImGui::TableHeadersRow();

        for (auto const& application : Applications)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(application.name);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("ImGui shell | logging | storage");
            ImGui::TableNextColumn();
            ImGui::TextDisabled("None");
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Recommendation: show future integrations here with direction, data type, and health.");
    ImGui::End();
}
}

void Watchlist::Gui::ShowDetachableAppWorkspace(
    DeviceMonitorState& deviceMonitorState, AppState* mqttState, bool* showConsole)
{
    static WorkspaceState state;
    DetachableAppContext const context{deviceMonitorState, mqttState};

    ShowApplicationRail(state, context, showConsole);
    ShowCentralApplication(state, context);
    ShowDetachedApplications(state, context);
    ShowApplicationMap(state);
}
