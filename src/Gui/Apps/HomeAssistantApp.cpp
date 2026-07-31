#include "HomeAssistantApp.hpp"

#include <imgui.h>

void Watchlist::Gui::ShowHomeAssistantAppContent(DetachableAppContext const&)
{
    static bool livingRoomLights = true;
    static bool nightMode = false;
    static float targetTemperature = 21.5f;

    ImGui::TextUnformatted("Home Assistant controller");
    ImGui::TextDisabled("Concept preview - controls are intentionally local and are not connected yet.");
    ImGui::Spacing();

    ImGui::Checkbox("Living room lights", &livingRoomLights);
    ImGui::Checkbox("Night mode", &nightMode);
    ImGui::SetNextItemWidth(240.0f);
    ImGui::SliderFloat("Target temperature", &targetTemperature, 16.0f, 28.0f, "%.1f C");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextDisabled("CONNECTION");
    ImGui::TextColored(ImVec4(1.0f, 0.72f, 0.25f, 1.0f), "Not configured");
}
