#include <chrono>
#include <filesystem>
#include <format>
#include <future>

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

    // Keep bounded request workers separate from long-lived services so
    // shutdown can drain work and acknowledgements before MQTT disconnects.
    Concurrency::ThreadPoolManager workerRuntime(3);
    Concurrency::ThreadPoolManager serviceRuntime(0);
    Messaging::MqttService mqtt(&appState);
    Dispatch::RequestDispatcher dispatcher(
        workerRuntime,
        std::filesystem::temp_directory_path() / "watchlist_server.sqlite",
        &appState,
        [&mqtt](const Messaging::MqttRequest& request, const Messaging::MqttAck& ack) {
            mqtt.Publish(Messaging::AckTopicForClient(request.sourceClientId), Messaging::SerializeAck(ack));
        });

    // The MQTT callback stays lightweight and hands validated work to RequestDispatcher.
    serviceRuntime.StartDedicatedThread("MQTT IO", [&mqtt, &appState, &dispatcher](std::stop_token stopToken) {
        auto consoleState = appState.SnapshotConsole();
        mqtt.Subscribe(Messaging::RequestsTopic, [&dispatcher](std::string, std::string payload) {
            [[maybe_unused]] const bool queued = dispatcher.HandleIncomingPayload(payload);
        });
        mqtt.Run(stopToken, consoleState.brokerHost, consoleState.brokerPort, consoleState.clientId);
    });

    std::promise<void> guiClosed;
    auto guiClosedFuture = guiClosed.get_future();
    serviceRuntime.StartDedicatedThread("ImGui", [&guiClosed](std::stop_token stopToken) {
        ImGuiStart(stopToken);
        guiClosed.set_value();
    });

    guiClosedFuture.wait();
    // Stop inbound delivery first, then drain queued worker work while the
    // transport remains connected for completion acknowledgements.
    dispatcher.StopAccepting();
    mqtt.Unsubscribe(Messaging::RequestsTopic);
    if (!mqtt.WaitForPendingOperations(std::chrono::seconds(3)))
        appState.AddActivity("Timed out waiting for MQTT request unsubscribe");

    workerRuntime.StopAll();
    workerRuntime.JoinAll();

    if (!mqtt.WaitForPendingOperations(std::chrono::seconds(3)))
        appState.AddActivity("Timed out waiting for MQTT acknowledgement publication");
    serviceRuntime.StopAll();
    serviceRuntime.JoinAll();
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
