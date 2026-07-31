#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Watchlist::Gui {

struct DeviceViewModel {
    std::string id;
    std::string name;
    std::string address;
    std::uint16_t port{};
    bool alive{};

    bool operator==(const DeviceViewModel&) const = default;
};

struct DeviceDraft {
    std::string id;
    std::string name;
    std::string address;
    int port{};
    bool alive{};
};

enum class DeviceCommandKind {
    Refresh,
    Add,
    Edit,
    Delete,
    SetAlive,
};

struct DeviceCommand {
    DeviceCommandKind kind{DeviceCommandKind::Refresh};
    std::string targetId;
    std::optional<DeviceViewModel> device;
};

enum class DeviceEditorMode {
    Add,
    Edit,
};

struct DeviceEditorState {
    DeviceEditorMode mode{DeviceEditorMode::Add};
    std::string originalId;
    DeviceDraft draft;
};

class DeviceMonitorState {
public:
    [[nodiscard]] const std::vector<DeviceViewModel>& Devices() const noexcept;
    [[nodiscard]] const std::optional<DeviceEditorState>& Editor() const noexcept;
    [[nodiscard]] DeviceEditorState* EditSession() noexcept;
    [[nodiscard]] const std::optional<std::string>& PendingDeleteId() const noexcept;
    [[nodiscard]] const std::optional<std::string>& Error() const noexcept;
    [[nodiscard]] bool ExitRequested() const noexcept;

    void ReplaceDevices(std::vector<DeviceViewModel> devices);
    void RequestRefresh();
    void BeginAdd();
    [[nodiscard]] bool BeginEdit(const std::string& id);
    void CancelEditor() noexcept;
    [[nodiscard]] bool SubmitEditor();
    [[nodiscard]] bool RequestDelete(const std::string& id);
    void CancelDelete() noexcept;
    [[nodiscard]] bool ConfirmDelete();
    [[nodiscard]] bool SetAlive(const std::string& id, bool alive);
    void RequestExit() noexcept;
    void SetError(std::string error);
    void ClearError() noexcept;
    [[nodiscard]] std::vector<DeviceCommand> ConsumeCommands();

private:
    [[nodiscard]] std::vector<DeviceViewModel>::iterator FindDevice(const std::string& id);
    [[nodiscard]] std::vector<DeviceViewModel>::const_iterator FindDevice(const std::string& id) const;
    [[nodiscard]] bool ValidateDraft(const DeviceEditorState& editor);

    std::vector<DeviceViewModel> devices_;
    std::optional<DeviceEditorState> editor_;
    std::optional<std::string> pendingDeleteId_;
    std::optional<std::string> error_;
    std::vector<DeviceCommand> commands_;
    bool exitRequested_{};
};

} // namespace Watchlist::Gui
