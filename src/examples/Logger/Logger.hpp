//All the declarations

#include <string>
#include <chrono>
#include <source_location>
#include <iostream>

void Log(LogLevel, std::string_view, std::source_location);


enum class LogLevel : char // TODO: place std::string here
{
    Info = 'I',
    Warning = 'W',
    Error = 'E',
};

//? --------------------------------------------------- what exactly const do?
auto LocalTime(std::chrono::system_clock::time_point const tp) 
{
    //return std::chrono::zoned_time{ std::chrono::current_zone(), tp };
    return std::chrono::current_zone();
}

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

      
    

/*
Zdenek's logger example

inline void Log(Fmt&& format, Args&&... args)
{
  fmt::print("[{}] {}\n", GetCurrentTime(),
      fmt::vformat(std::forward<Fmt>(format), fmt::make_format_args(std::forward<Args>(args)...)));
}

Log("Thread '{}' started", mName);

*/