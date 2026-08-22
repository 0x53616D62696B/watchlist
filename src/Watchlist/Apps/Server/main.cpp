#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <thread>
#include <utility>

#include "Common/Version.hpp"
#include "src/Gui/Gui.hpp"
#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Watchlist/AppState.hpp"
#include "src/Watchlist/DeviceStorageService.hpp"
#include "src/Watchlist/Dispatch/RequestDispatcher.hpp"
#include "src/Watchlist/Messaging/MqttMessages.hpp"
#include "src/Watchlist/Messaging/MqttService.hpp"
#include "src/Watchlist/Messaging/MqttTopics.hpp"

namespace Watchlist::Server {

int Run() noexcept
try
{
    LOG_INFO(std::format("WatchlistServer Version: {}", VERSION_FULL));

    AppState appState;
    auto console = appState.SnapshotConsole();
    console.clientId = "watchlist-server";
    appState.UpdateConsole(console);

    const auto databasePath =
        std::filesystem::temp_directory_path() / "watchlist_server.sqlite";
    Messaging::MqttService mqtt(&appState);
    std::optional<std::jthread> mqttThread;

    Gui::GuiResult guiResult;
    {
        auto workerRuntime = std::make_unique<Concurrency::ThreadPoolManager>(3);
        auto dispatcher = std::make_unique<Dispatch::RequestDispatcher>(
            *workerRuntime,
            databasePath,
            &appState,
            [&mqtt](const Messaging::MqttRequest& request, const Messaging::MqttAck& ack) {
                mqtt.Publish(
                    Messaging::AckTopicForClient(request.sourceClientId),
                    Messaging::SerializeAck(ack));
            });

        mqttThread.emplace([&mqtt, &appState, dispatcher = dispatcher.get()](std::stop_token stopToken) {
            const auto consoleState = appState.SnapshotConsole();
            mqtt.Subscribe(
                Messaging::RequestsTopic,
                [dispatcher](std::string, std::string payload) {
                    static_cast<void>(dispatcher->HandleIncomingPayload(payload));
                });
            mqtt.Run(
                stopToken,
                consoleState.brokerHost,
                consoleState.brokerPort,
                consoleState.clientId);
        });

        Gui::DeviceMonitorState deviceMonitorState;
        DeviceStorageService storage(databasePath);
        guiResult = Gui::ImGuiStart(
            deviceMonitorState,
            storage,
            std::this_thread::get_id(),
            {},
            &appState);

        dispatcher->StopAccepting();
        mqtt.Unsubscribe(Messaging::RequestsTopic);
        if (!mqtt.WaitForPendingOperations(std::chrono::seconds(3)))
            appState.AddActivity("Timed out waiting for MQTT request unsubscribe");

        // Queued work captures the dispatcher. Drain the fixed pool while the
        // dispatcher and MQTT acknowledgement publisher are still alive.
        workerRuntime.reset();
        dispatcher.reset();
    }

    if (!mqtt.WaitForPendingOperations(std::chrono::seconds(3)))
        appState.AddActivity("Timed out waiting for MQTT acknowledgement publication");
    mqttThread->request_stop();
    mqttThread->join();

    if (!guiResult.success)
    {
        LOG_ERROR(guiResult.error);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
catch (const std::exception& exception)
{
    LOG_EXCEPTION(exception);
    return EXIT_FAILURE;
}
catch (...)
{
    LOG_ERROR("Unknown WatchlistServer failure.");
    return EXIT_FAILURE;
}

} // namespace Watchlist::Server

int main()
{
    return Watchlist::Server::Run();
}
