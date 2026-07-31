#include "src/Gui/MyApp.hpp"

#include "src/Gui/ApplicationLogWindow.hpp"
#include "src/Gui/Workspace/DetachableAppWorkspace.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

#include <imgui.h>
#include <imgui_internal.h>

namespace Watchlist::Gui {
namespace {

void ShowViewportDockSpace()
{
    ImGuiViewport const* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowBgAlpha(0.0F);

    constexpr ImGuiWindowFlags hostWindowFlags = ImGuiWindowFlags_NoDocking |
                                                 ImGuiWindowFlags_NoTitleBar |
                                                 ImGuiWindowFlags_NoCollapse |
                                                 ImGuiWindowFlags_NoResize |
                                                 ImGuiWindowFlags_NoMove |
                                                 ImGuiWindowFlags_NoBringToFrontOnFocus |
                                                 ImGuiWindowFlags_NoNavFocus |
                                                 ImGuiWindowFlags_NoBackground;
    constexpr ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0F, 0.0F));
    ImGui::Begin("Watchlist DockSpace Host", nullptr, hostWindowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID const dockspaceId = ImGui::GetID("Watchlist Workspace DockSpace v2");
    if (ImGui::DockBuilderGetNode(dockspaceId) == nullptr)
    {
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodePos(dockspaceId, viewport->WorkPos);
        ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

        ImGuiID centralDockId = dockspaceId;
        ImGuiID const bottomDockId =
            ImGui::DockBuilderSplitNode(centralDockId, ImGuiDir_Down, 0.27F, nullptr, &centralDockId);
        ImGuiID const leftDockId =
            ImGui::DockBuilderSplitNode(centralDockId, ImGuiDir_Left, 0.19F, nullptr, &centralDockId);
        ImGui::DockBuilderDockWindow(ApplicationRailWindowName, leftDockId);
        ImGui::DockBuilderDockWindow(ApplicationContentWindowName, centralDockId);
        ImGui::DockBuilderDockWindow(ApplicationConsoleWindowName, bottomDockId);
        ImGui::DockBuilderFinish(dockspaceId);
    }

    ImGui::DockSpace(dockspaceId, ImVec2(0.0F, 0.0F), dockspaceFlags);
    ImGui::End();
}

} // namespace

void ShowWindow(DeviceMonitorState& state, AppState* mqttState)
{
    PROFILE_FUNCTION;
    if (state.ExitRequested())
        return;

    static bool showApplicationLog = true;
    if (ImGui::IsKeyPressed(ImGuiKey_F12, false))
        showApplicationLog = !showApplicationLog;

    ShowViewportDockSpace();
    ShowDetachableAppWorkspace(state, mqttState, &showApplicationLog);

    if (showApplicationLog)
        ShowApplicationLog(&showApplicationLog);
}

} // namespace Watchlist::Gui
