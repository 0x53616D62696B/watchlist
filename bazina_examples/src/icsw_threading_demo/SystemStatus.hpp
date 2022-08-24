#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <fmt/format.h>
#include "Utils.hpp"

class SystemStatus
{
public:
  void StartMonitoring()
  {
    mThread.Start([this]() { MonitoringLoop(); });
  }

  void StopMonitoring() { mThread.Stop(); }

  double GetHighVacuumPressure() const { return mHighVacuumPressure; }
  double GetCIDGasPressure() const { return mCIDGasPressure; }
  double GetTotalIonCurrent() const { return mTotalIonCurrent; }

  void SetHighVacuumPressure(double highVacuumPressure) { mHighVacuumPressure = highVacuumPressure; }
  void SetCIDGasPressure(double cidGasPressure) { mCIDGasPressure = cidGasPressure; }
  void SetTotalIonCurrent(double totalIonCurrent) { mTotalIonCurrent = totalIonCurrent; }

private:
  ThreadLoop mThread{"Monitoring loop"};
  double mHighVacuumPressure = 0.278;
  double mCIDGasPressure = 3.267;
  double mTotalIonCurrent = 1.35e-7;

  void MonitoringLoop();
};
