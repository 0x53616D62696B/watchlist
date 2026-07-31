#pragma once

#include <format>
#include <source_location>
#include <string_view>

enum class LogLevel : char
{
    Info = 'I',
    Warning = 'W',
    Error = 'E',
    Fatal = 'F',
    Debug = 'D',
    Trace = 'T',
};

// A call emits and flushes one complete newline-terminated record before it
// returns. Records from concurrent callers never interleave.
void Log(LogLevel level, std::string_view message,
    std::source_location source = std::source_location::current());

[[noreturn]] void LogFatal(std::string_view message,
    std::source_location source = std::source_location::current());

#define LOG_INFO(...) Log(LogLevel::Info, __VA_ARGS__)
#define LOG_WARNING(...) Log(LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) Log(LogLevel::Error, __VA_ARGS__)
#define LOG_FATAL(...) LogFatal(__VA_ARGS__)
#define LOG_DEBUG(...) Log(LogLevel::Debug, __VA_ARGS__)
#define LOG_TRACE(...) Log(LogLevel::Trace, __VA_ARGS__)
#define LOG_EXCEPTION(e) Log(LogLevel::Error, std::format("Exception error: {}", (e).what()))
