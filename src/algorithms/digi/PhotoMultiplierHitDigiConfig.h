// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto

#pragma once

#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/unit_system.h>
#include <spdlog/spdlog.h>

namespace eicrecon {
class PhotoMultiplierHitDigiConfig {
public:
  // Detector and readout identifiers
  std::string detectorName{""};
  std::string readoutClass{""};

  // triggering: compared to sim hit times, so in edm4eic units
  // - time gate in which input hits are grouped to 1 output hit, from the earliest hit
  double hitTimeWindow = 20 * edm4eic::unit::ns;
  // - time resolution (= 1 / TDC counts per unit time)
  double timeResolution = edm4eic::unit::ns / 16;
  double speMean        = 80.0;  // mean ADC counts for a single photon
  double speError       = 16.0;  // sigma of ADC counts for a single photon
  double pedMean        = 200.0; // mean ADC counts for the pedestal
  double pedError       = 3.0;   // sigma of ADC counts for the pedestal

  // noise: dark hits per pixel, uniformly distributed in [0, noiseTimeWindow)
  // - both values are in dd4hep units (time unit: second), so their product is dimensionless
  bool enableNoise       = false;
  double noiseRate       = 20 * dd4hep::kilohertz; // per pixel
  double noiseTimeWindow = 20 * dd4hep::ns;

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

inline std::ostream& operator<<(std::ostream& os, const PhotoMultiplierHitDigiConfig& cfg) {
  os << std::format("{:=^60}", " PhotoMultiplierHitDigiConfig Settings ") << std::endl;
  auto print_param = [&os](auto name, auto val) {
    os << std::format("  {:>20} = {:<}", name, val) << std::endl;
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
  os << std::format("{:-^60}", " Quantum Efficiency vs. Wavelength ") << std::endl;
  for (auto& [wl, qe] : cfg.quantumEfficiency)
    os << std::format("  {:>10} {:<}", wl, qe) << std::endl;
  return os;
}

} // namespace eicrecon
