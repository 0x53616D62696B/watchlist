#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Logger/LoggerTesting.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <barrier>
#include <cstddef>
#include <ostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>
#include <vector>

namespace
{
class LoggerReset final
{
public:
    ~LoggerReset()
    {
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
            {
                LOG_INFO(std::format("tag-{}-{}", thread, record));
            }
        });
    }
    threads.clear();

    std::istringstream records{output.str()};
    std::vector<std::string> lines;
    for (std::string line; std::getline(records, line);)
    {
        lines.push_back(std::move(line));
    }

    ASSERT_EQ(lines.size(), threadCount * recordsPerThread);
    for (std::size_t thread = 0; thread < threadCount; ++thread)
    {
        for (std::size_t record = 0; record < recordsPerThread; ++record)
        {
            auto const tag = std::format("tag-{}-{}", thread, record);
            EXPECT_EQ(std::ranges::count_if(lines,
                          [&](std::string const& line) { return line.ends_with(tag); }),
                1)
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
