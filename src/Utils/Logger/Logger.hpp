#pragma once
#include <string>
#include <chrono>
#include <source_location>
#include <iostream>
#include <filesystem>
#include <format>
//All the declarations

//TODO implement colored logger, which will be on only if terminal supports it
#define COLORED_LOG false


enum class LogLevel : char // TODO: place std::string here
{
    Info = 'I',
    Warning = 'W',
    Error = 'E',
    Fatal = 'F',
    Debug = 'D',
    Trace = 'T',
};

auto LocalTime(std::chrono::system_clock::time_point const);
std::string ToString(std::source_location const);
//void Log(LogLevel const, std::string_view const, std::source_location const);
void Log(LogLevel const level, std::string_view const message,
         std::source_location const source = std::source_location::current()); //TODO enter the third parameter as optional parameter


#define LOG_INFO(...) Log(LogLevel::Info, __VA_ARGS__)
#define LOG_WARNING(...) Log(LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) Log(LogLevel::Error, __VA_ARGS__)
#define LOG_FATAL(...) Log(LogLevel::Fatal, __VA_ARGS__)
#define LOG_DEBUG(...) Log(LogLevel::Debug, __VA_ARGS__)
#define LOG_TRACE(...) Log(LogLevel::Trace, __VA_ARGS__)

//TODO create Logger Class instea of logger funtion