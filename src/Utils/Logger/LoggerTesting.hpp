#pragma once

#if !defined(WATCHLIST_LOGGER_TESTING)
#error "LoggerTesting.hpp is available only to the logger test target"
#endif

#include <ostream>

namespace LoggerTesting
{
using FatalHandler = void (*)();

// The caller owns output and must keep it alive until ResetOutput() returns.
void SetOutput(std::ostream& output) noexcept;
void ResetOutput() noexcept;

// A handler may throw so tests can observe the otherwise non-returning path.
void SetFatalHandler(FatalHandler handler) noexcept;
void ResetFatalHandler() noexcept;
}
