#include "Logger.hpp"

#if defined(WATCHLIST_LOGGER_TESTING)
#include "LoggerTesting.hpp"
#endif

#include <chrono>
#include <cstdio>
#include <exception>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>

namespace
{
struct LoggerState
{
    std::mutex mutex;
    std::ostream* output = &std::cout;
#if defined(WATCHLIST_LOGGER_TESTING)
    LoggerTesting::FatalHandler fatalHandler = nullptr;
#endif
};

LoggerState& State()
{
    static LoggerState state;
    return state;
}

std::string_view Filename(std::string_view path) noexcept
{
    auto const separator = path.find_last_of("/\\");
    return separator == std::string_view::npos ? path : path.substr(separator + 1);
}

std::string BuildRecord(LogLevel level, std::string_view message, std::source_location source)
{
    return std::format("[{}] {} | {}:{} | {}\n", static_cast<char>(level),
        std::chrono::system_clock::now(), Filename(source.file_name()), source.line(), message);
}

void EmitRecord(std::string_view record)
{
    auto& state = State();
    std::scoped_lock lock{state.mutex};
    state.output->write(record.data(), static_cast<std::streamsize>(record.size()));
    state.output->flush();
}

void EmitFormattingFailure() noexcept
{
    constexpr std::string_view fallback = "[E] logger failed to format record\n";
    try
    {
        EmitRecord(fallback);
    }
    catch (...)
    {
        auto& state = State();
        std::scoped_lock lock{state.mutex};
        std::fwrite(fallback.data(), 1, fallback.size(), stderr);
        std::fflush(stderr);
    }
}

void EmitLogRecord(LogLevel level, std::string_view message, std::source_location source) noexcept
{
    try
    {
        EmitRecord(BuildRecord(level, message, source));
    }
    catch (...)
    {
        EmitFormattingFailure();
    }
}

[[noreturn]] void TerminateProcess()
{
#if defined(WATCHLIST_LOGGER_TESTING)
    LoggerTesting::FatalHandler handler = nullptr;
    {
        auto& state = State();
        std::scoped_lock lock{state.mutex};
        handler = state.fatalHandler;
    }
    if (handler != nullptr)
    {
        handler();
    }
#endif
    std::terminate();
}
}

void Log(LogLevel level, std::string_view message, std::source_location source)
{
    if (level == LogLevel::Fatal)
    {
        LogFatal(message, source);
    }
    EmitLogRecord(level, message, source);
}

[[noreturn]] void LogFatal(std::string_view message, std::source_location source)
{
    EmitLogRecord(LogLevel::Fatal, message, source);
    TerminateProcess();
}

#if defined(WATCHLIST_LOGGER_TESTING)
namespace LoggerTesting
{
void SetOutput(std::ostream& output) noexcept
{
    auto& state = State();
    std::scoped_lock lock{state.mutex};
    state.output = &output;
}

void ResetOutput() noexcept
{
    SetOutput(std::cout);
}

void SetFatalHandler(FatalHandler handler) noexcept
{
    auto& state = State();
    std::scoped_lock lock{state.mutex};
    state.fatalHandler = handler;
}

void ResetFatalHandler() noexcept
{
    SetFatalHandler(nullptr);
}
}
#endif

#ifdef DEBUG_LOGGER
int main()
{
    Log(LogLevel::Info, "Logging from main thread");
    LOG_DEBUG("Added one debug message");
    return 0;
}
#endif
