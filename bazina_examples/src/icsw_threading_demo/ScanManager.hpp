#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <fmt/format.h>
#include "Utils.hpp"

class Instrument;

class ScanManager
{
public:
  ScanManager(Instrument& instrument) : mInstrument(instrument) {}

  void StartScanning()
  {
    mThread.Start([this]() { ScanLoop(); });
  }

  void StopScanning() { mThread.Stop(); }

private:
  ThreadLoop mThread{"Scan loop"};
  Instrument& mInstrument;

  void ScanLoop();
};
