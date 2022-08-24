#include "ScanManager.hpp"
#include "Instrument.hpp"

void ScanManager::ScanLoop()
{
  Log("Scanning...");
  static double highVacuumPressure = 1e-3;
  static double cidGasPressure = 2e-8;
  static double totalIonCurrent = 0;

  mInstrument.GetSystemStatus().SetHighVacuumPressure(highVacuumPressure -= 1e-4);
  mInstrument.GetSystemStatus().SetCIDGasPressure(cidGasPressure += 1e-6);
  mInstrument.GetSystemStatus().SetTotalIonCurrent(totalIonCurrent += 1e-7);
}
