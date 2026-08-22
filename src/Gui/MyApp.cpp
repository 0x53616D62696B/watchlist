#include "src/Gui/MyApp.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <array>
#include <cstdint>
#include <format>
#include <string>
#include <vector>

#include "src/Utils/Profiling/TracyProfiling.hpp"
#include "src/Watchlist/AppState.hpp"
#include "src/Watchlist/Messaging/MqttMessages.hpp"

namespace Watchlist::Gui {
namespace {

void ShowError(DeviceMonitorState& state)
{
    if (!state.Error())
        return;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0F, 0.4F, 0.4F, 1.0F));
    ImGui::TextWrapped("%s", state.Error()->c_str());
    ImGui::PopStyleColor();
    ImGui::SameLine();
    if (ImGui::SmallButton("Dismiss"))
        state.ClearError();
}

void ShowEditor(DeviceMonitorState& state)
{
    auto* editor = state.EditSession();
    if (editor == nullptr)
        return;

    const bool adding = editor->mode == DeviceEditorMode::Add;
    const char* title = adding ? "Add device" : "Edit device";
    ImGui::OpenPopup(title);
    if (!ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return;

    if (!adding)
        ImGui::BeginDisabled();
    ImGui::InputText("ID", &editor->draft.id);
    if (!adding)
    {
        ImGui::EndDisabled();
        ImGui::TextDisabled("Device IDs are immutable after creation.");
    }
    ImGui::InputText("Name", &editor->draft.name);
    ImGui::InputText("Address", &editor->draft.address);
    ImGui::InputInt("Port", &editor->draft.port);
    ImGui::Checkbox("Alive", &editor->draft.alive);

    if (ImGui::Button(adding ? "Add" : "Save") && state.SubmitEditor())
        ImGui::CloseCurrentPopup();
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        state.CancelEditor();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void ShowDeleteConfirmation(DeviceMonitorState& state)
{
    if (!state.PendingDeleteId())
        return;

    ImGui::OpenPopup("Delete device?");
    if (!ImGui::BeginPopupModal("Delete device?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return;

    ImGui::TextWrapped("Delete device '%s'? This action cannot be undone.", state.PendingDeleteId()->c_str());
    if (ImGui::Button("Delete"))
    {
        static_cast<void>(state.ConfirmDelete());
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        state.CancelDelete();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void ShowDeviceTable(DeviceMonitorState& state)
{
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;
    if (!ImGui::BeginTable("devices", 6, flags))
        return;

    ImGui::TableSetupColumn("ID");
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Address");
    ImGui::TableSetupColumn("Port");
    ImGui::TableSetupColumn("Alive");
    ImGui::TableSetupColumn("Actions");
    ImGui::TableHeadersRow();

    for (const auto& device : state.Devices())
    {
        ImGui::PushID(device.id.c_str());
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(device.id.c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(device.name.c_str());
        ImGui::TableSetColumnIndex(2);
        ImGui::TextUnformatted(device.address.c_str());
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%u", static_cast<unsigned int>(device.port));
        ImGui::TableSetColumnIndex(4);
        bool alive = device.alive;
        if (ImGui::Checkbox("##alive", &alive))
            static_cast<void>(state.SetAlive(device.id, alive));
        ImGui::TableSetColumnIndex(5);
        if (ImGui::SmallButton("Edit"))
            static_cast<void>(state.BeginEdit(device.id));
        ImGui::SameLine();
        if (ImGui::SmallButton("Delete"))
            static_cast<void>(state.RequestDelete(device.id));
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void ShowHistory(const char* title, const std::vector<std::string>& lines)
{
    ImGui::TextUnformatted(title);
    ImGui::BeginChild(title, ImVec2(0.0F, 120.0F), true);
    for (const auto& line : lines)
        ImGui::TextWrapped("%s", line.c_str());
    ImGui::EndChild();
}

void ShowMqttConsole(AppState& appState)
{
    auto console = appState.SnapshotConsole();
    ImGui::Text("Status: %s", console.connectionStatus.c_str());
    ImGui::InputText("Broker", &console.brokerHost);
    ImGui::InputInt("Port", &console.brokerPort);
    ImGui::InputText("Client ID", &console.clientId);

    static constexpr std::array messageTypeLabels{
        "execute_script",
        "send_to_device",
        "cmd_to_device",
        "store_database_value",
        "query_AI_prompt",
    };
    ImGui::Combo(
        "Type",
        &console.selectedMessageType,
        messageTypeLabels.data(),
        static_cast<int>(messageTypeLabels.size()));

    switch (console.selectedMessageType)
    {
    case 0:
        ImGui::InputText("Script", &console.scriptName);
        break;
    case 1:
        ImGui::InputText("Device", &console.deviceId);
        ImGui::InputTextMultiline("Payload", &console.payload, ImVec2(-1.0F, 90.0F));
        break;
    case 2:
        ImGui::InputText("Device", &console.deviceId);
        ImGui::InputText("Command", &console.command);
        break;
    case 3:
        ImGui::InputText("Key", &console.key);
        ImGui::InputText("Value", &console.value);
        break;
    case 4:
        ImGui::InputTextMultiline("Prompt", &console.prompt, ImVec2(-1.0F, 90.0F));
        break;
    default:
        break;
    }
    appState.UpdateConsoleForm(console);

    if (ImGui::Button("Send MQTT request"))
    {
        static std::uint64_t requestCounter{};
        Messaging::MqttRequest request;
        request.id = std::format("{}-{}", console.clientId, ++requestCounter);
        request.type = static_cast<Messaging::MessageType>(console.selectedMessageType);
        request.createdUtc = Messaging::UtcNowIso8601();
        request.sourceClientId = console.clientId;
        request.scriptName = console.scriptName;
        request.deviceId = console.deviceId;
        request.payload = console.payload;
        request.command = console.command;
        request.key = console.key;
        request.value = console.value;
        request.prompt = console.prompt;
        static_cast<void>(appState.DispatchOutbound(Messaging::SerializeRequest(request)));
    }

    ShowHistory("Outbound requests", appState.Outbound());
    ShowHistory("Acknowledgements", appState.Acks());
    ShowHistory("MQTT activity", appState.Activity());
}

} // namespace

void ShowWindow(DeviceMonitorState& state, AppState* mqttState)
{
    PROFILE_FUNCTION;
    bool open = !state.ExitRequested();
    if (!open)
        return;

    if (!ImGui::Begin("Watchlist device monitor", &open, ImGuiWindowFlags_MenuBar))
    {
        ImGui::End();
        if (!open)
            state.RequestExit();
        return;
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit", "Alt+F4"))
                state.RequestExit();
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (ImGui::Button("Refresh"))
        state.RequestRefresh();
    ImGui::SameLine();
    if (ImGui::Button("Add device"))
        state.BeginAdd();

    if (mqttState != nullptr && ImGui::CollapsingHeader("MQTT console", ImGuiTreeNodeFlags_DefaultOpen))
        ShowMqttConsole(*mqttState);

    ShowError(state);
    ImGui::Separator();
    if (state.Devices().empty())
        ImGui::TextDisabled("No devices. Add a device or refresh from storage.");
    else
        ShowDeviceTable(state);

    ShowEditor(state);
    ShowDeleteConfirmation(state);
    ImGui::End();

    if (!open)
        state.RequestExit();
}

} // namespace Watchlist::Gui
