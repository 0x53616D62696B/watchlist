
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

// A dummy function
void foo(int Z)
{
    for (int i = 0; i < Z; i++) {
        std::cout << "Thread using function"
               " pointer as callable\n";
    }
}
  
// A callable object
class thread_obj {
public:
    void operator()(int x)
    {
        for (int i = 0; i < x; i++)
           std::cout << "Thread using function"
                  " object as  callable\n";
    }
};
  

int main()
{
    std::cout << "Threads 1 and 2 and 3 "
         "operating independently" << std::endl;
  
    // This thread is launched by using 
    // function pointer as callable
    /*
    std::thread th1(foo, 3);
  
    // This thread is launched by using
    // function object as callable
    std::thread th2(thread_obj(), 3);
  
    // Define a Lambda Expression
    auto f = [](int x) {
        for (int i = 0; i < x; i++)
            std::cout << "Thread using lambda"
             " expression as callable\n";
    };
  
    // This thread is launched by using 
    // lamda expression as callable
    std::thread th3(f, 3);
  
    // Wait for the threads to finish
    // Wait for thread t1 to finish
    th1.join();
  
    // Wait for thread t2 to finish
    th2.join();
  
    // Wait for thread t3 to finish
    th3.join();
    */

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
    const std::chrono::time_point tp = std::chrono::system_clock::now();

    std::cout << "Hello! Current time is: " << tp << std::endl;

    std::chrono::system_clock::time_point const tp2 = 
                        std::chrono::system_clock::now();


    const std::chrono::time_zone* tz = std::chrono::current_zone();
    std::string_view name = tz->name();
    
    // or auto
    //std::chrono::system_clock::time_point const tp3 = std::chrono::system_clock::now();
    //std::chrono::sys_time
    //std::chrono::zoned_time
    
    const std::chrono::zoned_time source{ std::chrono::current_zone(), std::chrono::system_clock::now() };
    //std::chrono::zoned_time source = std::chrono::zoned_time{               std::chrono::current_zone(), std::chrono::system_clock::now() };

    //std::string new_string1 = std::format("{:%F %T %Z}", source);
    std::string new_string1;
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
    
    //!: argument "source" is somehow wrong!
/*
    try {
        std::cout
            << std::format("{}", new_string1)
            << '\n';
        // << std::endl;
    } catch (std::chrono::nonexistent_local_time& ex) {
        std::cout << "Error: " << ex.what() << '\n';
    } catch (const std::exception& ex){
        std::cout << "Error: " << ex.what() << '\n';
    } catch (...) {
        std::cout << "Generic error occurred" << std::endl;
    }
*/    
    /*std::string new_string2 = std::format("{}:{}:{}",
        std::filesystem::path(source.file_name()).filename().string(),
        source.function_name(),
        source.line());
    */
    //Progress: LocalTIme funguje spravne.. takze je spatne ToString..
        //Co dela std::format? testnout jestli je to vhodnej argument..

    std::timespec ts;
    std::timespec_get(&ts, TIME_UTC);
    char buf[100];
    std::strftime(buf, sizeof buf, "%D %T", std::gmtime(&ts.tv_sec));
    std::cout << "Current time: " << buf << '.' << ts.tv_nsec << " UTC\n";




    return 0;
}