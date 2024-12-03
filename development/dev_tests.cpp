#include "../src/Utils/Logger/Logger.hpp"

#include <chrono>
#include <format>
#include <iostream>
#include <ctime>



#include <cstring>
#include <string>
#include <atomic>

//#include <cstdint> // Include the header file to define "u8"
// #include <cstddef> // Include the header file to define "u8"
// #include <sstream>
// #include <map>
// #include <set>
// #include <vector>

#include "import_this.hpp"
#include "dev_tests.hpp"



namespace DevTests{

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

void dev_tests_main(){

    print_compiler_version();

    //time_development_tests();

    std::string ip_address = "172.28.208.1";
    char* ip_address_char = new char[ip_address.length() + 1]; // Initialize the ip_address_char variable
    // std::strncpy(reinterpret_cast<char*>(ip_address_char), ip_address.c_str(), ip_address.length());
    strncpy_s(ip_address_char, ip_address.length() + 1, ip_address.c_str(), ip_address.length());
    //delete[] ip_address_char; // Deallocate the memory


    // std::string msg = "Hello, server!";
    // char* buffer_char = new char[msg.length() + 1]; // Initialize the buffer_char variable
    // std::strncpy(reinterpret_cast<char*>(buffer_char), msg.c_str(), msg.length());

    // const u8* test=reinterpret_cast<const u8*>(buffer_char);
    //DAQSock->Send(&buffer_char, msg.length(), true);


    
    //char retrieved_msg_char[1024]; // Initialize the retrieved_msg_char array
    


    //mBuffer.reserve(100);
    //size_t retrieved_msg_len = DAQSock->Receive(retrieved_msg_char, 1024, 1);
    const char* retrieved_msg_char = new char[1024]{};
    


    size_t retrieved_msg_len = 59;
    std::cout << "Reply from server: ";
    //retrieved_msg_char = new char[1024]{};
    std::cout.write(retrieved_msg_char, retrieved_msg_len);
    // std::cout.write(retrieved_msg_char[0], retrieved_msg_len);
    std::cout << std::endl;

    std::vector<std::uint8_t> mBuffer;
    mBuffer.reserve(100); // TODO not a member of a class! YET
    //size_t retrieved_msg_len = DAQSock->Receive(&mBuffer, 100, 1);
    std::cout << "Reply from server: " << &mBuffer << ". With msg length: " << retrieved_msg_len << std::endl;

    TestingSpace::import_this();


}

} // namespace DevTests
