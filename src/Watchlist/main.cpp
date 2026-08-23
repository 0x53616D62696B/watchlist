/**
 * @file Watchlist.cpp
 * @author Patrik Maraczek (https://github.com/0x53616D62696B)
 * @brief
 * @version 0.1
 * @date 2022-08-24
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <chrono>
#include <filesystem>
#include <format>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

#include "Common/Version.hpp"
#include "src/Gui/Gui.hpp"
#include "src/Utils/Concurrency/AsyncEventLoop.hpp"
#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Watchlist/Application.hpp"
#include "src/Watchlist/ApplicationPaths.hpp"
#include "src/Watchlist/AppState.hpp"
#include "src/Watchlist/DeviceStorageService.hpp"
#include "src/Watchlist/Messaging/MqttService.hpp"
#include "src/Watchlist/Messaging/MqttTopics.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

namespace Watchlist {
namespace {

std::string PathForLog(const std::filesystem::path& path)
{
    const auto utf8 = path.u8string();
    return {reinterpret_cast<const char*>(utf8.data()), utf8.size()};
}

/** Keeps short-lived profiling runs open when --wait-for-tracy is passed. */
void WaitForTracy()
{
    PROFILE_MESSAGE("[TRACY][MAIN] Watchlist waiting for Tracy");
    LOG_INFO("Watchlist is waiting so Tracy can connect. Press Enter to exit...");
    std::cin.get();
}

std::vector<Concurrency::AsyncEventLoop::Task> PopulateAsyncIoDemo(
    Concurrency::AsyncEventLoop& loop)
{
    std::vector<Concurrency::AsyncEventLoop::Task> tasks;
    tasks.reserve(7);

    tasks.push_back(loop.wait_for_event("application_started"));
    tasks.push_back(loop.wait_for_event("custom_event"));
    tasks.push_back(loop.wait_for_event("sensor_data_0"));
    tasks.push_back(loop.schedule_after(std::chrono::seconds(1), [] {
        LOG_INFO("AsyncIO demo one-second task executed.");
    }));
    tasks.push_back(loop.schedule_after(std::chrono::milliseconds(250), [] {
        LOG_INFO("AsyncIO demo 250ms task executed.");
    }));
    tasks.push_back(loop.schedule([] {
        LOG_INFO("AsyncIO demo immediate task executed.");
    }));

    auto eventStream = loop.create_event_stream("sensor_data", 5);
    tasks.push_back(loop.process_events(std::move(eventStream)));
    return tasks;
}

} // namespace

int RunApplication(int argc, char** argv) noexcept
{
    PROFILE_THREAD("Watchlist main");
    PROFILE_FUNCTION;
    std::vector<std::string_view> arguments;
    arguments.reserve(argc > 1 ? static_cast<std::size_t>(argc - 1) : 0U);
    for (int index = 1; index < argc; ++index)
        arguments.emplace_back(argv[index]);

    const ApplicationDependencies dependencies{
        .announceStartup = [] {
            LOG_INFO(std::format("Watchlist Version: {}", VERSION_FULL));
            PROFILE_THREAD("Watchlist main");
            PROFILE_FUNCTION;
            PROFILE_MESSAGE("[TRACY][MAIN] Watchlist application startup");
        },
        .waitForProfiler = WaitForTracy,
        .resolveDatabasePath = [argc, argv](std::span<const std::string_view>) {
            const auto path = ResolveDatabasePath(argc, argv);
            LOG_INFO(std::format("Watchlist database: {}", PathForLog(path)));
            return path;
        },
        .runGui = [](const std::filesystem::path& databasePath, bool hidden) {
            Gui::DeviceMonitorState deviceMonitorState;
            DeviceStorageService storage(
                databasePath, DeviceStorageService::ExecutionMode::CallingThread);
            AppState appState;
            Messaging::MqttService mqtt(&appState);
            Concurrency::AsyncEventLoop asyncIoDemo(
                Concurrency::AsyncEventLoop::ExecutionMode::CallingThread);
            Gui::GuiConfiguration guiConfiguration;
            guiConfiguration.hidden = hidden;
            if (hidden)
                deviceMonitorState.RequestExit();

            appState.SetOutboundCommandHandler([&mqtt](std::string payload) {
                mqtt.Publish(Messaging::RequestsTopic, std::move(payload));
            });

            PROFILE_SCOPE(ThreadPoolManagerLifetime);
            auto asyncIoDemoTasks = PopulateAsyncIoDemo(asyncIoDemo);
            LOG_DEBUG(std::format(
                "Application AsyncIO event loop populated with {} demo tasks.",
                asyncIoDemoTasks.size()));

            Concurrency::ThreadPoolManager threadPool(4);
            std::stop_source asyncIoStop;
            std::future<void> sqlFuture;
            std::future<void> messagingFuture;
            std::future<void> asyncIoFuture;
            std::future<Gui::GuiResult> guiFuture;
            try
            {
                sqlFuture = threadPool.enqueue([&storage] {
                    PROFILE_THREAD("Watchlist SQL");
                    PROFILE_SCOPE(ThreadPoolSQLite);
                    storage.RunOnCallingThread();
                });
                messagingFuture = threadPool.enqueue([&mqtt, &appState, stopToken = asyncIoStop.get_token()] {
                    PROFILE_THREAD("Watchlist MQTT I/O");
                    PROFILE_SCOPE(ThreadPoolMqttIO);
                    try
                    {
                        const auto console = appState.SnapshotConsole();
                        mqtt.Subscribe(
                            Messaging::AckTopicForClient(console.clientId),
                            [&appState](std::string, std::string payload) {
                                appState.AddAck(std::move(payload));
                            });
                        mqtt.Run(stopToken, console.brokerHost, console.brokerPort, console.clientId);
                    }
                    catch (const std::exception& exception)
                    {
                        LOG_EXCEPTION(exception);
                        appState.AddActivity(
                            std::string("Asynchronous messaging stopped: ") + exception.what());
                    }
                    catch (...)
                    {
                        LOG_ERROR("Asynchronous messaging stopped with an unknown error.");
                        appState.AddActivity(
                            "Asynchronous messaging stopped with an unknown error");
                    }
                });
                asyncIoFuture = threadPool.enqueue([&asyncIoDemo] {
                    PROFILE_THREAD("Watchlist AsyncIO demo");
                    PROFILE_SCOPE(ThreadPoolAsyncIODemo);
                    if (!asyncIoDemo.run_on_calling_thread())
                        throw std::runtime_error("AsyncIO event loop could not start on its pool worker");
                });
                guiFuture = threadPool.enqueue([&] {
                    PROFILE_THREAD("Watchlist GUI");
                    PROFILE_SCOPE(ThreadPoolImGui);
                    return Gui::ImGuiStart(
                        deviceMonitorState,
                        storage,
                        std::this_thread::get_id(),
                        guiConfiguration,
                        &appState);
                });
            }
            catch (...)
            {
                storage.RequestStop();
                asyncIoStop.request_stop();
                asyncIoDemo.stop();
                throw;
            }

            if (!asyncIoDemo.emit_event({
                    "application_started", std::string("Application worker pool started")}))
            {
                LOG_ERROR("AsyncIO demo rejected the application_started event.");
            }
            if (!asyncIoDemo.emit_event({
                    "custom_event", std::string("Hello from the Application AsyncIO thread")}))
            {
                LOG_ERROR("AsyncIO demo rejected the custom_event event.");
            }

            Gui::GuiResult result;
            try
            {
                result = guiFuture.get();
            }
            catch (...)
            {
                storage.RequestStop();
                asyncIoStop.request_stop();
                asyncIoDemo.stop();
                throw;
            }

            {
                PROFILE_SCOPE(ApplicationRequestWorkerStop);
                storage.RequestStop();
                asyncIoStop.request_stop();
                asyncIoDemo.stop();
            }
            {
                PROFILE_SCOPE(ApplicationJoinWorkers);
                sqlFuture.get();
                messagingFuture.get();
                asyncIoFuture.get();
            }
            appState.SetOutboundCommandHandler({});

            for (auto& storageResult : storage.PollResults())
            {
                if (!storageResult.success && result.success)
                    result = {.success = false, .error = std::move(storageResult.error)};
            }
            return ApplicationSubsystemResult{
                .success = result.success, .error = std::move(result.error)};
        },
    };
    return RunApplication(arguments, dependencies);
}

} // namespace Watchlist

int main(int argc, char** argv)
{
    return Watchlist::RunApplication(argc, argv);
}
