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

#include "src/Gui/Gui.hpp" //! How to make "Gui/Gui.hpp" work? 
// #include "Gui/Gui.hpp"
#include "src/Utils/Threading/ThreadPoolManager.hpp"
#include "src/Utils/Threading/EventLoopCoroutine.hpp"
#include "src/Utils/Threading/EventLoopGenerator.hpp"
#include "src/Utils/Threading/HybridEventLoop.hpp"

int main(int argc, char** argv)
try
{
    //* Multithreading Tests
    std::cout << std::format("Starting Multithreading examples") << std::endl;

    Threading::example_thread_pool_manager();
    std::cout << std::format("thread_pool DONE") << std::endl;

    Threading::example_eloop_gen();
    std::cout << std::format("main_eloop_gen DONE\n") << std::endl;

    Threading::example_eloop_coro();
    std::cout << std::format("main_eloop_coro DONE\n") << std::endl;

    Threading::example_eloop_hybrid();
    std::cout << std::format("main_eloop_hybrid DONE\n") << std::endl;
    //* Multithreading Tests END

    //! Ending code here for Testing purpose
    return EXIT_SUCCESS;
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