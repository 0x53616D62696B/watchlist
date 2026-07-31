#pragma once

#include <chrono>
#include <cstdint>
#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>

enum class LogLevel : char
{
    Info = 'I',
    Warning = 'W',
    Error = 'E',
    Fatal = 'F',
    Debug = 'D',
    Trace = 'T',
};

struct LogEntry
{
    std::uint64_t sequence;
    LogLevel level;
    std::chrono::system_clock::time_point timestamp;
    std::string source;
    std::string message;
};

// A call emits and flushes one complete newline-terminated record before it
// returns. Records from concurrent callers never interleave.
void Log(LogLevel level, std::string_view message,
    std::source_location source = std::source_location::current());

[[noreturn]] void LogFatal(std::string_view message,
    std::source_location source = std::source_location::current());

std::vector<LogEntry> GetLogEntriesSince(std::uint64_t sequence);
void ClearLogEntries();

#define LOG_INFO(...) Log(LogLevel::Info, __VA_ARGS__)
#define LOG_WARNING(...) Log(LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) Log(LogLevel::Error, __VA_ARGS__)
#define LOG_FATAL(...) LogFatal(__VA_ARGS__)
#define LOG_DEBUG(...) Log(LogLevel::Debug, __VA_ARGS__)
#define LOG_TRACE(...) Log(LogLevel::Trace, __VA_ARGS__)
#define LOG_EXCEPTION(e) Log(LogLevel::Error, std::format("Exception error: {}", (e).what()))
