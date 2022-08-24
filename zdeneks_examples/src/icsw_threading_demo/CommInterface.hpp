#pragma once
#include <fmt/format.h>
#include "Instrument.hpp"

class CommInterface
{
public:
  CommInterface(Instrument& instrument) : mInstrument(instrument) {}

  void StartListening()
  {
    mThread.Start([this]() { ListeningLoop(); });
  }

  void StopListening() { mThread.Stop(); }

private:
  ThreadLoop mThread{"Listening loop"};
  Instrument& mInstrument;

  void ListeningLoop()
  {
    const int message = std::rand() % 10; // simulate random incoming messages
    DispatchMessage(message);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  void DispatchMessage(int message)
  {
    switch (message)
    {
    case 0:
      Log(">>> Start system status monitoring");
      mInstrument.GetSystemStatus().StartMonitoring();
      break;
    case 1:
      Log(">>> Stop system status monitoring");
      mInstrument.GetSystemStatus().StopMonitoring();
      break;
    case 2:
      Log(">>> Start scanning");
      mInstrument.GetScanManager().StartScanning();
      break;
    case 3:
      Log(">>> Stop scanning");
      mInstrument.GetScanManager().StopScanning();
      break;
    case 4:
      Log(">>> Get high vacuum pressure");
      Log("High vacuum pressure: {:.1e}", mInstrument.GetSystemStatus().GetHighVacuumPressure());
      break;
    case 5:
      Log(">>> Get CID gas pressure");
      Log("CID gas pressure: {:.1e}", mInstrument.GetSystemStatus().GetCIDGasPressure());
      break;
    case 6:
      Log(">>> Get total ion current");
      Log("Total ion current: {:.1e}", mInstrument.GetSystemStatus().GetTotalIonCurrent());
      break;
    default:
      break;
    }
  }
};
