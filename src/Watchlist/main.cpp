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

#include "src/Gui/Gui.hpp" //! How to make "Gui/Gui.hpp" work? 
// #include "Gui/Gui.hpp"
#include "src/Utils/Threading/ThreadPoolManager.hpp"

int main(int argc, char** argv)
try
{
    //! In progress: Init Multithreading pool
    // Initialize ThreadPoolManager with 4 threads
    Threading::ThreadPoolManager threadPool(4);

    // Example: Enqueue tasks
    auto future1 = threadPool.enqueue([] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return "Task 1 completed";
    });

    auto future2 = threadPool.enqueue([] {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return "Task 2 completed";
    });

    // Retrieve results
    std::cout << future1.get() << std::endl;
    std::cout << future2.get() << std::endl;

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