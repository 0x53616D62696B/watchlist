#include "src/Gui/DeviceMonitorState.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace Watchlist::Gui {

const std::vector<DeviceViewModel>& DeviceMonitorState::Devices() const noexcept { return devices_; }
const std::optional<DeviceEditorState>& DeviceMonitorState::Editor() const noexcept { return editor_; }
DeviceEditorState* DeviceMonitorState::EditSession() noexcept { return editor_ ? &*editor_ : nullptr; }
const std::optional<std::string>& DeviceMonitorState::PendingDeleteId() const noexcept { return pendingDeleteId_; }
const std::optional<std::string>& DeviceMonitorState::Error() const noexcept { return error_; }
bool DeviceMonitorState::ExitRequested() const noexcept { return exitRequested_; }

void DeviceMonitorState::ReplaceDevices(std::vector<DeviceViewModel> devices)
{
    devices_ = std::move(devices);
    error_.reset();
}

void DeviceMonitorState::RequestRefresh()
{
    commands_.push_back({.kind = DeviceCommandKind::Refresh});
}

void DeviceMonitorState::BeginAdd()
{
    error_.reset();
    editor_ = DeviceEditorState{.mode = DeviceEditorMode::Add};
}

bool DeviceMonitorState::BeginEdit(const std::string& id)
{
    const auto found = FindDevice(id);
    if (found == devices_.end())
    {
        SetError("The selected device no longer exists.");
        return false;
    }

    error_.reset();
    editor_ = DeviceEditorState{
        .mode = DeviceEditorMode::Edit,
        .originalId = found->id,
        .draft = {.id = found->id, .name = found->name, .address = found->address, .port = found->port, .alive = found->alive},
    };
    return true;
}

void DeviceMonitorState::CancelEditor() noexcept { editor_.reset(); }

bool DeviceMonitorState::SubmitEditor()
{
    if (!editor_)
        return false;
    if (!ValidateDraft(*editor_))
        return false;

    const DeviceViewModel device{
        .id = editor_->draft.id,
        .name = editor_->draft.name,
        .address = editor_->draft.address,
        .port = static_cast<std::uint16_t>(editor_->draft.port),
        .alive = editor_->draft.alive,
    };

    if (editor_->mode == DeviceEditorMode::Add)
    {
        if (FindDevice(device.id) != devices_.end())
        {
            SetError("A device with this ID already exists.");
            return false;
        }
        devices_.push_back(device);
        commands_.push_back({.kind = DeviceCommandKind::Add, .targetId = device.id, .device = device});
    }
    else
    {
        if (device.id != editor_->originalId)
        {
            SetError("A device ID cannot be changed after creation.");
            return false;
        }
        auto found = FindDevice(editor_->originalId);
        if (found == devices_.end())
        {
            SetError("The device being edited no longer exists.");
            return false;
        }
        *found = device;
        commands_.push_back({.kind = DeviceCommandKind::Edit, .targetId = device.id, .device = device});
    }

    editor_.reset();
    error_.reset();
    return true;
}

bool DeviceMonitorState::RequestDelete(const std::string& id)
{
    if (FindDevice(id) == devices_.end())
    {
        SetError("The selected device no longer exists.");
        return false;
    }
    pendingDeleteId_ = id;
    return true;
}

void DeviceMonitorState::CancelDelete() noexcept { pendingDeleteId_.reset(); }

bool DeviceMonitorState::ConfirmDelete()
{
    if (!pendingDeleteId_)
        return false;

    const std::string id = *pendingDeleteId_;
    const auto found = FindDevice(id);
    if (found == devices_.end())
    {
        pendingDeleteId_.reset();
        SetError("The device selected for deletion no longer exists.");
        return false;
    }

    devices_.erase(found);
    commands_.push_back({.kind = DeviceCommandKind::Delete, .targetId = id});
    pendingDeleteId_.reset();
    error_.reset();
    return true;
}

bool DeviceMonitorState::SetAlive(const std::string& id, bool alive)
{
    auto found = FindDevice(id);
    if (found == devices_.end())
    {
        SetError("The selected device no longer exists.");
        return false;
    }
    found->alive = alive;
    commands_.push_back({.kind = DeviceCommandKind::SetAlive, .targetId = id, .device = *found});
    error_.reset();
    return true;
}

void DeviceMonitorState::RequestExit() noexcept { exitRequested_ = true; }

void DeviceMonitorState::SetError(std::string error) { error_ = std::move(error); }
void DeviceMonitorState::ClearError() noexcept { error_.reset(); }

std::vector<DeviceCommand> DeviceMonitorState::ConsumeCommands()
{
    return std::exchange(commands_, {});
}

std::vector<DeviceViewModel>::iterator DeviceMonitorState::FindDevice(const std::string& id)
{
    return std::find_if(devices_.begin(), devices_.end(), [&id](const DeviceViewModel& device) { return device.id == id; });
}

std::vector<DeviceViewModel>::const_iterator DeviceMonitorState::FindDevice(const std::string& id) const
{
    return std::find_if(devices_.cbegin(), devices_.cend(), [&id](const DeviceViewModel& device) { return device.id == id; });
}

bool DeviceMonitorState::ValidateDraft(const DeviceEditorState& editor)
{
    if (editor.draft.id.empty() || editor.draft.name.empty() || editor.draft.address.empty())
    {
        SetError("ID, name, and address are required.");
        return false;
    }
    if (editor.draft.port < 0 || editor.draft.port > std::numeric_limits<std::uint16_t>::max())
    {
        SetError("Port must be between 0 and 65535.");
        return false;
    }
    return true;
}

} // namespace Watchlist::Gui
