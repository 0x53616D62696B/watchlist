#include <format>
#include <iostream>

#include "src/Common/Version.hpp"
#include "src/Gui/Gui.hpp"
#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Watchlist/AppState.hpp"
#include "src/Watchlist/Messaging/MqttService.hpp"
#include "src/Watchlist/Messaging/MqttTopics.hpp"

namespace Watchlist::Client {

int Run()
try
{
    LOG_INFO(std::format("WatchlistClient Version: {}", VERSION_FULL));

    AppState appState;
    SetAppState(&appState);

    Concurrency::ThreadPoolManager runtime(2);
    Messaging::MqttService mqtt(&appState);

    auto console = appState.SnapshotConsole();
    console.clientId = "watchlist-client";
    appState.UpdateConsole(console);

    // MQTT has its own service thread so reconnect/subscription work never blocks ImGui frames.
    runtime.StartDedicatedThread("MQTT IO", [&mqtt, &appState](std::stop_token stopToken) {
        auto consoleState = appState.SnapshotConsole();
        mqtt.Subscribe(Messaging::AckTopicForClient(consoleState.clientId), [&appState](std::string, std::string payload) {
            appState.AddAck(std::move(payload));
        });
        mqtt.Run(stopToken, consoleState.brokerHost, consoleState.brokerPort, consoleState.clientId);
    });

    runtime.StartDedicatedThread("ImGui", [&runtime](std::stop_token stopToken) {
        ImGuiStart(stopToken);
        // Closing the GUI is the user-facing shutdown signal for the current apps.
        runtime.StopAll();
    });

    runtime.JoinAll();
    SetAppState(nullptr);
    return EXIT_SUCCESS;
}
catch (const std::exception& exception)
{
    LOG_EXCEPTION(exception);
    return EXIT_FAILURE;
}

} // namespace Watchlist::Client

int main()
{
    return Watchlist::Client::Run();
}
