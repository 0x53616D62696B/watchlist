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

#include <format>
#include <iostream>
#include <string_view>

#include "src/Common/Version.hpp"
#include "src/Gui/Gui.hpp" //! How to make "Gui/Gui.hpp" work? 
// #include "Gui/Gui.hpp"
#include "src/Watchlist/SQLiteThreadWorkerExample.hpp"
#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Utils/Concurrency/AsyncEventLoop.hpp"

namespace
{
/** Returns true when the exact command-line argument is present. */
bool HasArgument(int argc, char** argv, std::string_view argument)
{
    for (int index = 1; index < argc; ++index)
    {
        if (std::string_view(argv[index]) == argument)
            return true;
    }

    return false;
}

/** Keeps short-lived profiling runs open when --wait-for-tracy is passed. */
void WaitForTracyIfRequested(int argc, char** argv)
{
    if (!HasArgument(argc, argv, "--wait-for-tracy"))
        return;

    PROFILE_MESSAGE("[TRACY][MAIN] Watchlist waiting for Tracy");
    LOG_INFO("Watchlist is waiting so Tracy can connect. Press Enter to exit...");
    std::cin.get();
}
}

int main(int argc, char** argv)
try
{
    WaitForTracyIfRequested(argc, argv);

    LOG_INFO(std::format("Watchlist Version: {}", VERSION_FULL));

    PROFILE_THREAD("Watchlist main");
    PROFILE_FUNCTION;
    PROFILE_MESSAGE("[TRACY][MAIN] Watchlist application startup");

    /** Thread Pool Manger
     */
    PROFILE_SCOPE(ThreadPoolManagerLifetime);
    Concurrency::ThreadPoolManager threadPool(3); // Create a thread pool with 3 threads.
    
    /** Thread Pool Manager - 1.st worker thread - ImGui
     *  - Could be separated into multiple? For example separate: New Frame, WatchlistUI, RenderFrame 
     * into multiple threads?
     */
    auto futureImGui = threadPool.enqueue([] {
        PROFILE_SCOPE(ThreadPoolImGui);
        PROFILE_MESSAGE("[TRACY][THREAD_POOL] ImGui thread starts");
        ImGuiStart();
        // No return needed
    });

    /** Thread Pool Manager - 2.nd worker thread - AsyncIO thread for some quick awaitable calls, for example waiting 
    * for response from MQTT.
    */
    auto futureAsyncIOThread = threadPool.enqueue([] {
        PROFILE_SCOPE(ThreadPoolAsyncIO);
        PROFILE_MESSAGE("[TRACY][THREAD_POOL] AsyncIO thread starts");
        Concurrency::AsyncEventLoop asyncLoop;

        // Wait for a quit AsyncLoop event
        Concurrency::AsyncEventLoop::Task quitAsyncLoopEvent = asyncLoop.wait_for_event("async_loop_quit");

        // How to emit this event.
        //TODO: We should be able to emit this event anywhere, even outside this thread..??
        asyncLoop.emit_event({"async_loop_quit", std::string("Async loop quit event")});

        // TODO: I do not have any task yet. But I would like to be able to add tasks to this asyncLoop whener in code in future. How to make this loop to await anything?

        // No return needed
    });

    /** Thread Pool Manager - 3.rd worker thread - SQLiteCpp database example.
     */
    auto SQLiteCppThread = threadPool.enqueue([] {
        PROFILE_SCOPE(ThreadPoolSQLiteCpp);
        PROFILE_MESSAGE("[TRACY][THREAD_POOL] SQLiteCpp thread starts");
        Concurrency::run_sqlitecpp_thread_worker_example();
        return "SQLiteCpp thread completed.";
    });

    // Testing Threads futures results
    // LOG_INFO(futureImGui.get()); // Output: Task 1 completed
    // LOG_INFO(futureAsyncIOThread.get()); // Output: Task 2 completed
    LOG_DEBUG(SQLiteCppThread.get());

    PROFILE_MESSAGE("[TRACY][THREAD_POOL] Done: futures returned, pool destructor will stop workers");

    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    LOG_EXCEPTION(e);
    return EXIT_FAILURE;
}
catch (...)
{
    LOG_ERROR("Unknown error in main.");
    return EXIT_FAILURE;
}
