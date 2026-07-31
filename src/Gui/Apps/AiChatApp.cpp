#include "AiChatApp.hpp"

#include <imgui.h>

#include <array>

void Watchlist::Gui::ShowAiChatAppContent(DetachableAppContext const&)
{
    static std::array<char, 1024> prompt{};

    ImGui::TextUnformatted("AI chat controller");
    ImGui::TextDisabled("Concept preview - keep each provider adapter behind this independent view.");
    ImGui::Spacing();

    if (ImGui::BeginChild("ChatPreview", ImVec2(0.0f, -90.0f), true))
    {
        ImGui::TextDisabled("ASSISTANT");
        ImGui::TextWrapped("This detachable surface can host a conversation without coupling it to the other apps.");
    }
    ImGui::EndChild();

    ImGui::SetNextItemWidth(-75.0f);
    ImGui::InputTextWithHint("##AiChatPrompt", "Message...", prompt.data(), prompt.size());
    ImGui::SameLine();
    ImGui::BeginDisabled(prompt.front() == '\0');
    if (ImGui::Button("Send", ImVec2(-1.0f, 0.0f)))
        prompt.front() = '\0';
    ImGui::EndDisabled();
}
