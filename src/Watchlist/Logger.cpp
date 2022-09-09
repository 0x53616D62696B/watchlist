// All the definitions
#include "Logger.hpp"


void Log(LogLevel const level,
        std::string_view const message,
        std::source_location const source = std::source_location::current())
{
    std::cout
        << std::format("[{}] {} | {} | {}",
                     static_cast<char>(level), 
                     to_string(as_local(std::chrono::system_clock::now())), 
                     to_string(source), 
                     message)
        << '\n';
        // << std::endl;
}  