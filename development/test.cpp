#include "../src/Utils/Logger/Logger.hpp"

#include <chrono>
#include <format>
#include <iostream>
#include <ctime>

#include "test.hpp"



void time_development_tests(){
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
}

void print_compiler_version(){

    if (_MSC_VER){ //msvc
        std::cout << "MSVC Compiler ";
        if (_MSC_VER >= 1920) std::cout << "C++20";
        else if (_MSC_VER >= 1910) std::cout << "C++17";
        else if (_MSC_VER >= 1900) std::cout << "C++14";
        else if (_MSC_VER >= 1800) std::cout << "C++11";
        else if (_MSC_VER >= 1500) std::cout << "C++98";
        else std::cout << "pre-standard C++.";
        std::cout << "\n";
    }
    else { //gcc
        std::cout << "GCC Compiler ";
        if (__cplusplus == 202101L) std::cout << "C++23";
        else if (__cplusplus == 202002L) std::cout << "C++20";
        else if (__cplusplus == 201703L) std::cout << "C++17";
        else if (__cplusplus == 201402L) std::cout << "C++14";
        else if (__cplusplus == 201103L) std::cout << "C++11";
        else if (__cplusplus == 199711L) std::cout << "C++98";
        else std::cout << "pre-standard C++." << __cplusplus;
        std::cout << "\n";
    }   
}

int main(int argc, char* argv[]){
    if (argc!= 2){
    }

    print_compiler_version();

    //time_development_tests();

    std::string ip_address = "172.28.208.1";
    char* ip_address_char = new char[ip_address.length() + 1]; // Initialize the ip_address_char variable
    // std::strncpy(reinterpret_cast<char*>(ip_address_char), ip_address.c_str(), ip_address.length());
    strncpy_s(ip_address_char, ip_address.length() + 1, ip_address.c_str(), ip_address.length());
    //delete[] ip_address_char; // Deallocate the memory

    return 0;
}
