#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Logger/LoggerTesting.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <barrier>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace
{
class LoggerReset final
{
public:
    LoggerReset()
    {
        ClearLogEntries();
    }

    ~LoggerReset()
    {
        ClearLogEntries();
        LoggerTesting::ResetFatalHandler();
        LoggerTesting::ResetOutput();
    }
};

class FlushCountingBuffer final : public std::stringbuf
{
public:
    int sync() override
    {
        ++flushCount;
        return std::stringbuf::sync();
    }

    int flushCount = 0;
};

struct FatalSignal
{
};

void ThrowFatalSignal()
{
    throw FatalSignal{};
}

TEST(LoggerTests, ConcurrentRecordsRemainCompleteLines)
{
    LoggerReset reset;
    std::ostringstream output;
    LoggerTesting::SetOutput(output);

    constexpr std::size_t threadCount = 8;
    constexpr std::size_t recordsPerThread = 100;
    std::barrier start{static_cast<std::ptrdiff_t>(threadCount)};
    std::vector<std::jthread> threads;
    threads.reserve(threadCount);

    for (std::size_t thread = 0; thread < threadCount; ++thread)
    {
        threads.emplace_back([&, thread] {
            start.arrive_and_wait();
            for (std::size_t record = 0; record < recordsPerThread; ++record)
                LOG_INFO(std::format("tag-{}-{}", thread, record));
        });
    }
    threads.clear();

    std::istringstream records{output.str()};
    std::vector<std::string> lines;
    for (std::string line; std::getline(records, line);)
        lines.push_back(std::move(line));

    ASSERT_EQ(lines.size(), threadCount * recordsPerThread);
    for (std::size_t thread = 0; thread < threadCount; ++thread)
    {
        for (std::size_t record = 0; record < recordsPerThread; ++record)
        {
            auto const tag = std::format("tag-{}-{}", thread, record);
            EXPECT_EQ(std::ranges::count_if(lines, [&](std::string const& line) { return line.ends_with(tag); }), 1)
                << tag;
        }
    }
}

TEST(LoggerTests, FatalRecordIsFlushedBeforeTerminationHandler)
{
    LoggerReset reset;
    FlushCountingBuffer buffer;
    std::ostream output{&buffer};
    LoggerTesting::SetOutput(output);
    LoggerTesting::SetFatalHandler(&ThrowFatalSignal);

    EXPECT_THROW(LOG_FATAL("fatal-tag"), FatalSignal);
    EXPECT_EQ(buffer.flushCount, 1);
    EXPECT_TRUE(buffer.str().ends_with("fatal-tag\n"));
    auto const entries = GetLogEntriesSince(0);
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.front().level, LogLevel::Fatal);
}

TEST(LoggerTests, NonFatalRecordIsFlushedBeforeLogReturns)
{
    LoggerReset reset;
    FlushCountingBuffer buffer;
    std::ostream output{&buffer};
    LoggerTesting::SetOutput(output);

    LOG_INFO("flushed-tag");

    EXPECT_EQ(buffer.flushCount, 1);
    EXPECT_TRUE(buffer.str().ends_with("flushed-tag\n"));
}

TEST(LoggerTests, DirectFatalLevelUsesFatalContract)
{
    LoggerReset reset;
    std::ostringstream output;
    LoggerTesting::SetOutput(output);
    LoggerTesting::SetFatalHandler(&ThrowFatalSignal);

    EXPECT_THROW(Log(LogLevel::Fatal, "direct-fatal"), FatalSignal);
    EXPECT_TRUE(output.str().ends_with("direct-fatal\n"));
}

TEST(LoggerTests, CapturesStructuredEntryAndSeverity)
{
    LoggerReset reset;
    std::ostringstream output;
    LoggerTesting::SetOutput(output);

    LOG_WARNING("structured message");
    auto const entries = GetLogEntriesSince(0);

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.front().level, LogLevel::Warning);
    EXPECT_EQ(entries.front().message, "structured message");
    EXPECT_NE(entries.front().source.find("LoggerTests.cpp"), std::string::npos);
    EXPECT_GT(entries.front().sequence, 0);
}

TEST(LoggerTests, IncrementalReadsOnlyReturnNewerEntries)
{
    LoggerReset reset;
    std::ostringstream output;
    LoggerTesting::SetOutput(output);

    LOG_INFO("first");
    auto const firstRead = GetLogEntriesSince(0);
    ASSERT_EQ(firstRead.size(), 1);

    LOG_ERROR("second");
    auto const incrementalRead = GetLogEntriesSince(firstRead.front().sequence);

    ASSERT_EQ(incrementalRead.size(), 1);
    EXPECT_EQ(incrementalRead.front().message, "second");
    EXPECT_GT(incrementalRead.front().sequence, firstRead.front().sequence);
}

TEST(LoggerTests, PreservesEverySeverityLevel)
{
    LoggerReset reset;
    std::ostringstream output;
    LoggerTesting::SetOutput(output);

    LOG_INFO("info");
    LOG_WARNING("warning");
    LOG_ERROR("error");
    LOG_DEBUG("debug");
    LOG_TRACE("trace");

    auto const entries = GetLogEntriesSince(0);
    ASSERT_EQ(entries.size(), 5);
    EXPECT_EQ(entries[0].level, LogLevel::Info);
    EXPECT_EQ(entries[1].level, LogLevel::Warning);
    EXPECT_EQ(entries[2].level, LogLevel::Error);
    EXPECT_EQ(entries[3].level, LogLevel::Debug);
    EXPECT_EQ(entries[4].level, LogLevel::Trace);
}

TEST(LoggerTests, ClearingRetainsMonotonicSequenceNumbers)
{
    LoggerReset reset;
    std::ostringstream output;
    LoggerTesting::SetOutput(output);

    LOG_DEBUG("before clear");
    auto const beforeClear = GetLogEntriesSince(0);
    ASSERT_EQ(beforeClear.size(), 1);

    ClearLogEntries();
    EXPECT_TRUE(GetLogEntriesSince(0).empty());

    LOG_TRACE("after clear");
    auto const afterClear = GetLogEntriesSince(beforeClear.front().sequence);

    ASSERT_EQ(afterClear.size(), 1);
    EXPECT_EQ(afterClear.front().message, "after clear");
    EXPECT_GT(afterClear.front().sequence, beforeClear.front().sequence);
}

TEST(LoggerTests, ConcurrentProducersHaveUniqueMonotonicOrdering)
{
    LoggerReset reset;
    std::ostringstream output;
    LoggerTesting::SetOutput(output);

    constexpr int threadCount = 8;
    constexpr int messagesPerThread = 50;
    std::vector<std::thread> producers;
    producers.reserve(threadCount);
    for (int threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        producers.emplace_back([threadIndex] {
            for (int messageIndex = 0; messageIndex < messagesPerThread; ++messageIndex)
                LOG_INFO(std::to_string(threadIndex) + ":" + std::to_string(messageIndex));
        });
    }
    for (auto& producer : producers)
        producer.join();

    auto const entries = GetLogEntriesSince(0);
    ASSERT_EQ(entries.size(), threadCount * messagesPerThread);
    for (std::size_t index = 1; index < entries.size(); ++index)
        EXPECT_EQ(entries[index].sequence, entries[index - 1].sequence + 1);
}

TEST(LoggerTests, EvictsEntriesOlderThanHistoryLimit)
{
    LoggerReset reset;
    std::ostringstream output;
    LoggerTesting::SetOutput(output);

    constexpr int extraEntries = 5;
    for (int index = 0; index < 10'000 + extraEntries; ++index)
        LOG_TRACE(std::to_string(index));

    auto const entries = GetLogEntriesSince(0);
    ASSERT_EQ(entries.size(), 10'000);
    EXPECT_EQ(entries.front().message, std::to_string(extraEntries));
    EXPECT_EQ(entries.back().message, "10004");
    for (std::size_t index = 1; index < entries.size(); ++index)
        EXPECT_GT(entries[index].sequence, entries[index - 1].sequence);
}

TEST(LoggerDeathTest, DefaultFatalHandlerTerminates)
{
    EXPECT_DEATH_IF_SUPPORTED(
        {
            LoggerTesting::ResetFatalHandler();
            LOG_FATAL("default-fatal");
        },
        "");
}
}
