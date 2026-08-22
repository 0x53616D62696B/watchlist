#include <cstdlib>
#include <filesystem>
#include <format>
#include <thread>
#include <utility>

#include "Common/Version.hpp"
#include "src/Gui/Gui.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Watchlist/AppState.hpp"
#include "src/Watchlist/DeviceStorageService.hpp"
#include "src/Watchlist/Messaging/MqttService.hpp"
#include "src/Watchlist/Messaging/MqttTopics.hpp"

namespace Watchlist::Client {

int Run() noexcept
try
{
    LOG_INFO(std::format("WatchlistClient Version: {}", VERSION_FULL));

    AppState appState;
    auto console = appState.SnapshotConsole();
    console.clientId = "watchlist-client";
    appState.UpdateConsole(console);

    Messaging::MqttService mqtt(&appState);
    appState.SetOutboundCommandHandler([&mqtt](std::string payload) {
        mqtt.Publish(Messaging::RequestsTopic, std::move(payload));
    });

    std::jthread mqttThread([&mqtt, &appState](std::stop_token stopToken) {
        const auto consoleState = appState.SnapshotConsole();
        mqtt.Subscribe(
            Messaging::AckTopicForClient(consoleState.clientId),
            [&appState](std::string, std::string payload) {
                appState.AddAck(std::move(payload));
            });
        mqtt.Run(
            stopToken,
            consoleState.brokerHost,
            consoleState.brokerPort,
            consoleState.clientId);
    });

    Gui::DeviceMonitorState deviceMonitorState;
    DeviceStorageService storage(
        std::filesystem::temp_directory_path() / "watchlist_client.sqlite");
    const auto guiResult = Gui::ImGuiStart(
        deviceMonitorState,
        storage,
        std::this_thread::get_id(),
        {},
        &appState);

    mqttThread.request_stop();
    mqttThread.join();
    appState.SetOutboundCommandHandler({});
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
    LOG_ERROR("Unknown WatchlistClient failure.");
    return EXIT_FAILURE;
}

} // namespace Watchlist::Client

int main()
{
    return Watchlist::Client::Run();
}
