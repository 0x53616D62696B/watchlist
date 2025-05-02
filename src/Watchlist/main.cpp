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

int main(int argc, char** argv)
try
{
    //TODO Init Multithreading pool

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