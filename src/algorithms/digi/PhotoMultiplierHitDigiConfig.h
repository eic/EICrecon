// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto

#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {
class PhotoMultiplierHitDigiConfig {
public:
  // Detector and readout identifiers
  std::string detectorName{""};
  std::string readoutClass{""};

  // triggering
  double hitTimeWindow =
      20.0; // time gate in which 2 input hits will be grouped to 1 output hit // [ns]
  double timeResolution = 1 / 16.0; // time resolution (= 1 / TDC counts per unit time) // [ns]
  double speMean        = 80.0;     // mean ADC counts for a single photon
  double speError       = 16.0;     // sigma of ADC counts for a single photon
  double pedMean        = 200.0;    // mean ADC counts for the pedestal
  double pedError       = 3.0;      // sigma of ADC counts for the pedestal

  // noise
  bool enableNoise       = false;
  double noiseRate       = 20000; // [Hz]
  double noiseTimeWindow = 20.0;  // [ns]

  // SiPM pixels
  bool enablePixelGaps = false; // enable/disable removal of hits in gaps between pixels

  // overall safety factor
  /* simulations assume the detector is ideal and perfect, but reality is
       * often neither; use this safety factor to reduce the number of initial
       * photon hits for a more conservative estimate of the number of
       * photoelectrons, or set to 1 to apply no such factor
       */
  double safetyFactor = 1.0; // allowed range: (0,1]

  // quantum efficiency
  bool enableQuantumEfficiency = true;
  // - wavelength units are [nm]
  // FIXME: figure out how users can override this, maybe an external `yaml` file
  std::vector<std::pair<double, double>> quantumEfficiency = {
      {315, 0.00}, {325, 0.04}, {340, 0.10}, {350, 0.20}, {370, 0.30}, {400, 0.35},
      {450, 0.40}, {500, 0.38}, {550, 0.35}, {600, 0.27}, {650, 0.20}, {700, 0.15},
      {750, 0.12}, {800, 0.08}, {850, 0.06}, {900, 0.04}, {1000, 0.00}};

  /*
         std::vector<std::pair<double, double> > quantumEfficiency = { // test unit QE
         {325, 1.00},
         {900, 1.00}
         };
         */

  friend std::ostream& operator<<(std::ostream& os, const PhotoMultiplierHitDigiConfig& cfg);
};

std::ostream& operator<<(std::ostream& os, const PhotoMultiplierHitDigiConfig& cfg) {
  os << fmt::format("{:=^60}", " PhotoMultiplierHitDigiConfig Settings ") << std::endl;
  auto print_param = [&os](auto name, auto val) {
    os << fmt::format("  {:>20} = {:<}", name, val) << std::endl;
  };
  print_param("hitTimeWindow", cfg.hitTimeWindow);
  print_param("timeResolution", cfg.timeResolution);
  print_param("speMean", cfg.speMean);
  print_param("speError", cfg.speError);
  print_param("pedMean", cfg.pedMean);
  print_param("pedError", cfg.pedError);
  print_param("enablePixelGaps", cfg.enablePixelGaps);
  print_param("safetyFactor", cfg.safetyFactor);
  print_param("enableNoise", cfg.enableNoise);
  print_param("noiseRate", cfg.noiseRate);
  print_param("noiseTimeWindow", cfg.noiseTimeWindow);
  os << fmt::format("{:-^60}", " Quantum Efficiency vs. Wavelength ") << std::endl;
  for (auto& [wl, qe] : cfg.quantumEfficiency)
    os << fmt::format("  {:>10} {:<}", wl, qe) << std::endl;
  return os;
}

} // namespace eicrecon
