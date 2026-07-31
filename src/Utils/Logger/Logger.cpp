#include "Logger.hpp"

#if defined(WATCHLIST_LOGGER_TESTING)
#include "LoggerTesting.hpp"
#endif

#include <chrono>
#include <cstdio>
#include <deque>
#include <exception>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>
#include <utility>

namespace
{
constexpr std::size_t MaxLogEntries = 10'000;

struct LoggerState
{
    std::mutex mutex;
    std::ostream* output = &std::cout;
    std::deque<LogEntry> entries;
    std::uint64_t nextSequence = 1;
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

void EmitRecord(LogLevel level, std::string_view message, std::source_location source)
{
    auto const timestamp = std::chrono::system_clock::now();
    auto sourceName = std::format("{}:{}", Filename(source.file_name()), source.line());
    auto messageText = std::string(message);
    auto record = std::format("[{}] {} | {} | {}\n", static_cast<char>(level), timestamp, sourceName, messageText);

    auto& state = State();
    std::scoped_lock lock{state.mutex};
    LogEntry entry{
        .sequence = state.nextSequence,
        .level = level,
        .timestamp = timestamp,
        .source = std::move(sourceName),
        .message = std::move(messageText),
    };
    state.entries.push_back(std::move(entry));
    ++state.nextSequence;
    if (state.entries.size() > MaxLogEntries)
        state.entries.pop_front();

    state.output->write(record.data(), static_cast<std::streamsize>(record.size()));
    state.output->flush();
}

void EmitFormattingFailure() noexcept
{
    constexpr std::string_view fallback = "[E] logger failed to format record\n";
    try
    {
        auto& state = State();
        std::scoped_lock lock{state.mutex};
        state.output->write(fallback.data(), static_cast<std::streamsize>(fallback.size()));
        state.output->flush();
    }
    catch (...)
    {
        std::fwrite(fallback.data(), 1, fallback.size(), stderr);
        std::fflush(stderr);
    }
}

void EmitLogRecord(LogLevel level, std::string_view message, std::source_location source) noexcept
{
    try
    {
        EmitRecord(level, message, source);
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
        handler();
#endif
    std::terminate();
}
}

void Log(LogLevel level, std::string_view message, std::source_location source)
{
    if (level == LogLevel::Fatal)
        LogFatal(message, source);
    EmitLogRecord(level, message, source);
}

[[noreturn]] void LogFatal(std::string_view message, std::source_location source)
{
    EmitLogRecord(LogLevel::Fatal, message, source);
    TerminateProcess();
}

std::vector<LogEntry> GetLogEntriesSince(std::uint64_t sequence)
{
    auto& state = State();
    std::scoped_lock lock{state.mutex};

    std::vector<LogEntry> result;
    for (auto const& entry : state.entries)
    {
        if (entry.sequence > sequence)
            result.push_back(entry);
    }
    return result;
}

void ClearLogEntries()
{
    auto& state = State();
    std::scoped_lock lock{state.mutex};
    state.entries.clear();
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
