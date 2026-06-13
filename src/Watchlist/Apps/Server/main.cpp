#include <chrono>
#include <filesystem>
#include <format>
#include <thread>

#include "src/Common/Version.hpp"
#include "src/Gui/Gui.hpp"
#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Watchlist/AppState.hpp"
#include "src/Watchlist/Dispatch/RequestDispatcher.hpp"
#include "src/Watchlist/Messaging/MqttMessages.hpp"
#include "src/Watchlist/Messaging/MqttService.hpp"
#include "src/Watchlist/Messaging/MqttTopics.hpp"

namespace Watchlist::Server {

int Run()
try
{
    LOG_INFO(std::format("WatchlistServer Version: {}", VERSION_FULL));

    AppState appState;
    SetAppState(&appState);

    auto console = appState.SnapshotConsole();
    console.clientId = "watchlist-server";
    appState.UpdateConsole(console);

    Concurrency::ThreadPoolManager runtime(3);
    Messaging::MqttService mqtt(&appState);
    Dispatch::RequestDispatcher dispatcher(
        runtime,
        std::filesystem::temp_directory_path() / "watchlist_server.sqlite",
        &appState,
        [&mqtt](const Messaging::MqttRequest& request, const Messaging::MqttAck& ack) {
            mqtt.Publish(Messaging::AckTopicForClient(request.sourceClientId), Messaging::SerializeAck(ack));
        });

    // The MQTT callback stays lightweight and hands validated work to RequestDispatcher.
    runtime.StartDedicatedThread("MQTT IO", [&mqtt, &appState, &dispatcher](std::stop_token stopToken) {
        auto consoleState = appState.SnapshotConsole();
        mqtt.Connect(consoleState.brokerHost, consoleState.brokerPort, consoleState.clientId);
        mqtt.Subscribe(Messaging::RequestsTopic, [&dispatcher](std::string, std::string payload) {
            [[maybe_unused]] const auto ack = dispatcher.HandleIncomingPayload(payload);
        });

        while (!stopToken.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        mqtt.Disconnect();
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

} // namespace Watchlist::Server

int main()
{
    return Watchlist::Server::Run();
}
