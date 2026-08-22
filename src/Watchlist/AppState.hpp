#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace Watchlist {

/// Mutable form values shown by the MQTT Console UI.
struct ConsoleState {
    std::string brokerHost = "localhost";
    int brokerPort = 1883;
    std::string clientId = "watchlist-client";
    std::string connectionStatus = "Offline";
    int selectedMessageType = 0;
    std::string scriptName = "script1";
    std::string deviceId = "device-1";
    std::string payload = "hello";
    std::string command = "status";
    std::string key = "sample";
    std::string value = "42";
    std::string prompt = "Summarize device health";
};

/// Thread-safe UI event history shared by GUI, MQTT, and dispatcher threads.
class AppState {
public:
    using OutboundCommandHandler = std::function<void(std::string payload)>;

    ConsoleState SnapshotConsole() const;
    void UpdateConsole(const ConsoleState& console);
    /// Update editable GUI fields without overwriting transport-owned status.
    void UpdateConsoleForm(const ConsoleState& console);
    void SetOutboundCommandHandler(OutboundCommandHandler handler);
    [[nodiscard]] bool DispatchOutbound(std::string payload);
    void AddOutbound(std::string line);
    void AddReceived(std::string line);
    void AddAck(std::string line);
    void AddActivity(std::string line);
    [[nodiscard]] std::vector<std::string> Outbound() const;
    [[nodiscard]] std::vector<std::string> Received() const;
    [[nodiscard]] std::vector<std::string> Acks() const;
    [[nodiscard]] std::vector<std::string> Activity() const;

private:
    static constexpr std::size_t maxLines_ = 50;

    void AddBounded(std::vector<std::string>& lines, std::string line);

    mutable std::mutex mutex_;
    OutboundCommandHandler outboundCommandHandler_;
    ConsoleState console_;
    std::vector<std::string> outbound_;
    std::vector<std::string> received_;
    std::vector<std::string> acks_;
    std::vector<std::string> activity_;
};

} // namespace Watchlist
