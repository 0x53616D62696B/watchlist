// All the definitions

#include <string>
#include <chrono>
#include <source_location>
#include <iostream>
#include <ctime>

void Log(LogLevel, std::string_view, std::source_location);


enum class LogLevel : char // TODO: place std::string here
{
    Info = 'I',
    Warning = 'W',
    Error = 'E',
};

auto LocalTime()
{
    std::timespec ts;
    std::timespec_get(&ts, TIME_UTC);
    char buf[100];
    std::strftime(buf, sizeof buf, "%D %T", std::gmtime(&ts.tv_sec));
    std::cout << "Current time: " << buf << '.' << ts.tv_nsec << " UTC\n";

}
/*
//? --------------------------------------------------- what exactly const do?
auto LocalTime(std::chrono::system_clock::time_point const tp) 
{
    //return std::chrono::zoned_time{ std::chrono::current_zone(), tp };
    return std::chrono::current_zone();
}
*/
std::string ToString(auto tp)
{
    //return std::format("{:%F %T %Z}", tp.first, tp.second, tp.third);
    //? -------------------------------------------------- test tp.first etc
    return std::format("{:%F %T %Z}", tp,
        source.file_name(),
        source.function_name(),
        source.line()); //source.line_number();

    //NOTE: format types here: https://en.cppreference.com/w/cpp/chrono/system_clock/formatter#Format_specification   
        
}
void Log(LogLevel const level,
        std::string_view const message,
        std::source_location const source = std::source_location::current())
{
    std::cout
        << std::format("[{}] {} | {} | {}",
                       static_cast<char>(level),
                       ToString(LocalTime(std::chrono::system_clock::now())),
                       ToString(source),
                       message)
        << '\n';
    // << std::endl;
} 

void Execute(int, double)
{
    Log(LogLevel::Error, "Error in execure!");
}

int main()
{
    // Logger test
    Log(LogLevel::Info, "Logging from main thread");
    Execute(0, 0);

    return 0;
}