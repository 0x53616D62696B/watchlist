#include "../src/Utils/Logger/Logger.hpp"
#include <chrono>
#include <format>
#include <iostream>
#include <ctime>

int main(int argc, char* argv[]){
    if (argc!= 2){
    }

    Log(LogLevel::Info, "Logging from main thread");
    printf("Starting\n\n");
    std::chrono::time_point hello = std::chrono::system_clock::now();
    std::cout << std::format("{}", hello) << std::endl;
    // 2
    //std::cout << std::format("{}", (hello++10)) << std::endl;
    printf("\n\n");

    std::timespec ts;
    std::timespec_get(&ts, TIME_UTC);
    char buf[100];
    std::strftime(buf, sizeof buf, "%D %T", std::gmtime(&ts.tv_sec));
    std::cout << "Current time: " << buf << '.' << ts.tv_nsec << " UTC\n";

    return 0;
}