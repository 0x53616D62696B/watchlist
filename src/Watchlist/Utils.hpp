#pragma once
#include <thread>
#include <mutex>
#include <string>
#include <sstream>







class Thread
{
  public:
  Thread(const std::string& name) : mName(name) {}
  ~Thread() { Stop(); }

  void Stop()
  {
    if (not mRunning)
      return;

    mRunning = false;
  }
  private:
  std::atomic<bool> mRunning = false;
  //std::mutex mMutex;
  //std::jthread mThread;
  std::string mName;

};

/*
#include "libs/fmt/include/fmt/format.h"
#include "libs/fmt/include/fmt/chrono.h"
//#include <fmt/format.h>
//#include <fmt/chrono.h>

inline std::string GetCurrentThreadId()
{
  std::stringstream ss;
  ss << std::this_thread::get_id();
  const auto str = ss.str();
  return fmt::format("{}", str.c_str() + str.size() - 3);
}

inline std::string GetCurrentTime()
{
  return fmt::format("{:%H:%M:%S}", fmt::localtime(std::time(nullptr)));
}

template <typename Fmt, typename... Args>
inline void Log(Fmt&& format, Args&&... args)
{
  fmt::print("[{}][TID {}] {}\n", GetCurrentTime(), GetCurrentThreadId(),
      fmt::vformat(std::forward<Fmt>(format), fmt::make_format_args(std::forward<Args>(args)...)));
}

class ThreadLoop
{
public:
  ThreadLoop(const std::string& name) : mName(name) {}
  ~ThreadLoop() { Stop(); }

  template <typename F>
  void Start(F&& function)
  {
    if (mRunning)
      return;

    std::scoped_lock lock(mMutex);
    if (mThread.joinable())
      mThread.join();

    mRunning = true;
    mThread = std::jthread([this, function]() {
      Log("Thread '{}' started", mName);
      while (mRunning)
      {
        function();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }
      Log("Thread '{}' stopped", mName);
    });
  }

  void Stop()
  {
    if (not mRunning)
      return;

    mRunning = false;
  }

private:
  std::atomic<bool> mRunning = false;
  std::mutex mMutex;
  std::jthread mThread;
  std::string mName;
};
*/