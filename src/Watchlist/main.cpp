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

#include "src/Common/version.hpp"
#include "src/Gui/Gui.hpp" //! How to make "Gui/Gui.hpp" work? 
// #include "Gui/Gui.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"
#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Utils/Concurrency/EventLoopCoroutine.hpp"
#include "src/Utils/Concurrency/EventLoopGenerator.hpp"
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
    std::cout << "Watchlist is waiting so Tracy can connect. Press Enter to exit..." << std::endl;
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

    //* Concurrency Examples
    {
        PROFILE_SCOPE(ConcurrencyExamples);
        
        std::cout << std::format("Starting Concurrency examples") << std::endl;

        PROFILE_MESSAGE("[TRACY][CONCURRENCY] Thread Pool Manager example starting");
        Concurrency::example_thread_pool_manager();
        std::cout << std::format("thread_pool DONE") << std::endl;

        PROFILE_MESSAGE("[TRACY][CONCURRENCY] ELoop Generator example starting");
        Concurrency::example_eloop_gen();
        std::cout << std::format("main_eloop_gen DONE\n") << std::endl;

        PROFILE_MESSAGE("[TRACY][CONCURRENCY] ELoop Coroutine example starting");
        Concurrency::example_eloop_coro();
        std::cout << std::format("main_eloop_coro DONE\n") << std::endl;

        PROFILE_MESSAGE("[TRACY][CONCURRENCY] Async ELoop example starting");
        Concurrency::example_async_eloop();
        std::cout << std::format("main_eloop_hybrid DONE\n") << std::endl;
    }

    PROFILE_MESSAGE("[TRACY][MAIN] Watchlist startup example finished");
    //return EXIT_SUCCESS;
    //TODO Thread Async MQTT

    //TODO Thread MQTT processing

    // Thread GUI
    ImGuiStart();
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
