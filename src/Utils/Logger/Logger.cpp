#include "Logger.hpp"

// TODO: swap auto to std::chrono::zoned_time... or smthing this metod returns.
//! std::chrono::zoned_time doesnt work as return type.. check documentation
auto LocalTime(std::chrono::system_clock::time_point const tp)
{
  return std::chrono::zoned_time{std::chrono::current_zone(), tp};
}

std::string ToString(std::source_location const source)
{
  // return std::format("{:%F %T %Z}", tp.first, tp.second, tp.third);
  //? -------------------------------------------------- test tp.first etc
  return std::format("{}:{}:{}",
      // source.file_name(),
      std::filesystem::path(source.file_name()).filename().string(), source.function_name(), source.line());

  // * format types here: https://en.cppreference.com/w/cpp/chrono/system_clock/formatter#Format_specification
}
void Log(LogLevel const level, std::string_view const message, std::source_location const source)
{
  // std::source_location const source = std::source_location::current();
  try
  {
    std::cout << std::format("[{}] {} | {} | {}",
                     // Log severity level
                     static_cast<char>(level),
                     // ToString(LocalTimeToString(std::chrono::system_clock::now())), //TODO: This is not working.
                     // Viz this: std::string new_string1 = std::format("{:%F %T %Z}", std::chrono::zoned_time{
                     // std::chrono::current_zone(), std::chrono::system_clock::now() });
                     //  Time
                     std::format("{}", std::chrono::system_clock::now()),
                     // Source
                     ToString(source),
                     // Log Message
                     message)
              << '\n';
    // << std::endl;
  }
  catch (std::chrono::nonexistent_local_time& ex)
  {
    std::cout << "Error: " << ex.what() << '\n';
  }
  catch (const std::exception& ex)
  {
    std::cout << "Error: " << ex.what() << '\n';
  }
  catch (...)
  {
    std::cout << "Generic error occurred" << std::endl;
  }
}

// TODO set this definition inside cmake list. True if logger is building.
// #define DEBUG_LOGGER
#ifdef DEBUG_LOGGER
int main()
{
  // Logger test
  Log(LogLevel::Info, "Logging from main thread");
  LOG_DEBUG("Added one debug meessage");
  LOG_FATAL("\033[1;31mbold red text\033[0m\n");

  return 0;
}
#endif