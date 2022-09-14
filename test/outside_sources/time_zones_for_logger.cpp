
// CPP program to demonstrate multithreading
// using three different callables.
#include <iostream>
#include <thread>

#include <chrono>
#include <ctime>

#include <filesystem>
#include <format>
#include <string_view>
#include <source_location>
#include <string>
#include <string_view>


#include <stdexcept>
#include <iomanip>
#include <algorithm>
  

int main()
{
    // First example
    /*
    constexpr std::string_view locations[] = {
        "Africa/Casablanca",   "America/Argentina/Buenos_Aires",
        "America/Barbados",    "America/Indiana/Petersburg",
        "America/Tarasco_Bar", "Antarctica/Casey",
        "Antarctica/Vostok",   "Asia/Magadan",
        "Asia/Manila",         "Asia/Shanghai",
        "Asia/Tokyo",          "Atlantic/Bermuda",
        "Australia/Darwin",    "Europe/Isle_of_Man",
        "Europe/Laputa",       "Indian/Christmas",
        "Indian/Cocos",        "Pacific/Galapagos",
    };
    constexpr auto width = std::ranges::max_element(locations, {},
        [](const auto& s) { return s.length(); })->length();
 
    for (const auto location : locations) {
        try {
            // may throw if `location` is not in the time zone database
            const std::chrono::zoned_time zt{location, std::chrono::system_clock::now()};
            std::cout << std::setw(width) << location << " - Zoned Time: " << zt << '\n';
        } catch (std::chrono::nonexistent_local_time& ex) {
            std::cout << "Error: " << ex.what() << '\n';
        } catch (const std::exception& ex){
            std::cout << "Error: " << ex.what() << '\n';
        } catch (...) {
            std::cout << "Generic error occurred" << std::endl;
        }
    }
    */
    
    //Second example

    const std::chrono::time_point tp = std::chrono::system_clock::now();

    std::cout << "Hello! Current time is: " << tp << std::endl;

    std::chrono::system_clock::time_point const tp2 = 
                        std::chrono::system_clock::now();

    // Third example
    const std::chrono::time_zone* tz = std::chrono::current_zone();
    std::string_view name = tz->name();
   
    const std::chrono::zoned_time source{ std::chrono::current_zone(), std::chrono::system_clock::now() };

    //std::string new_string1 = std::format("{:%F %T %Z}", source);
    //std::string new_string1;
    try {
        //new_string1 = std::format("{0:%T}", source);
        std::cout
            << std::format("{}", tp)
            << '\n';
    } catch (std::chrono::nonexistent_local_time& ex) {
        std::cout << "Error: " << ex.what() << '\n';
    } catch (const std::exception& ex){
        std::cout << "Error: " << ex.what() << '\n';
    } catch (...) {
        std::cout << "Generic error occurred" << std::endl;
    }


    // Forth example
    std::timespec ts;
    std::timespec_get(&ts, TIME_UTC);
    char buf[100];
    std::strftime(buf, sizeof buf, "%D %T", std::gmtime(&ts.tv_sec));
    std::cout << "Current time: " << buf << '.' << ts.tv_nsec << " UTC\n";




    return 0;
}