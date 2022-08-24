#include "SystemStatus.hpp"

void SystemStatus::MonitoringLoop()
{
  Log("Monitoring system status...");
  mHighVacuumPressure = std::clamp(mHighVacuumPressure, 0., 1.);
  mCIDGasPressure = std::clamp(mCIDGasPressure, 0., 1.);
  mTotalIonCurrent = std::clamp(mTotalIonCurrent, 0., 1e-3);
}
