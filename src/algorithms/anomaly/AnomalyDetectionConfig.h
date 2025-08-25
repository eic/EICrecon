#pragma once

#include <string>
#include <vector>

namespace eicrecon {

struct AnomalyDetectionConfig {
  // Detector subsystems to monitor
  std::vector<std::string> detector_systems = {"BEMC", "BHCAL", "EEMC", "EHCAL", "FEMC",   "FHCAL",
                                               "BTRK", "ECTRK", "BVTX", "DRICH", "PFRICH", "DIRC",
                                               "BTOF", "ECTOF", "ZDC",  "B0TRK", "B0ECAL"};

  // Energy threshold for considering particles (GeV)
  double energy_threshold = 0.1;

  // Momentum threshold for considering particles (GeV/c)
  double momentum_threshold = 0.1;

  // Maximum anomaly value for normalization
  double max_anomaly_value = 10.0;

  // Update frequency for audio output (events)
  int update_frequency = 10;
};

} // namespace eicrecon
