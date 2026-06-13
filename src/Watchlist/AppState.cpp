#include "src/Watchlist/AppState.hpp"

#include <utility>

namespace Watchlist {
namespace {
AppState* g_appState = nullptr;
} // namespace

ConsoleState AppState::SnapshotConsole() const
{
    std::scoped_lock lock(mutex_);
    return console_;
}

void AppState::UpdateConsole(const ConsoleState& console)
{
    std::scoped_lock lock(mutex_);
    console_ = console;
}

void AppState::AddOutbound(std::string line)
{
    std::scoped_lock lock(mutex_);
    AddBounded(outbound_, std::move(line));
}

void AppState::AddReceived(std::string line)
{
    std::scoped_lock lock(mutex_);
    AddBounded(received_, std::move(line));
}

void AppState::AddAck(std::string line)
{
    std::scoped_lock lock(mutex_);
    AddBounded(acks_, std::move(line));
}

void AppState::AddActivity(std::string line)
{
    std::scoped_lock lock(mutex_);
    AddBounded(activity_, std::move(line));
}

std::vector<std::string> AppState::Outbound() const
{
    std::scoped_lock lock(mutex_);
    return outbound_;
}

std::vector<std::string> AppState::Received() const
{
    std::scoped_lock lock(mutex_);
    return received_;
}

std::vector<std::string> AppState::Acks() const
{
    std::scoped_lock lock(mutex_);
    return acks_;
}

std::vector<std::string> AppState::Activity() const
{
    std::scoped_lock lock(mutex_);
    return activity_;
}

void AppState::AddBounded(std::vector<std::string>& lines, std::string line)
{
    lines.push_back(std::move(line));
    if (lines.size() > maxLines_) {
        lines.erase(lines.begin(), lines.begin() + static_cast<std::ptrdiff_t>(lines.size() - maxLines_));
    }
}

void SetAppState(AppState* state)
{
    g_appState = state;
}

AppState* GetAppState()
{
    return g_appState;
}

} // namespace Watchlist
