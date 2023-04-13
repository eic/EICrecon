// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks

#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {
  class PhotoMultiplierHitDigiConfig {
    public:

      // random number generator seed
      // FIXME: don't use 0 if `TRandomMixMax` is the RNG, it can get "stuck"
      // FIXME: a warning will be printed if 0 is used; remove this warning when a better RNG is used
      unsigned long seed = 1; // seed for RNG (note: `0` might mean "unique" seed)

      // triggering
      double hitTimeWindow  = 20.0;   // time gate in which 2 input hits will be grouped to 1 output hit // [ns]
      double timeResolution = 1/16.0; // time resolution (= 1 / TDC counts per unit time) // [ns]
      double speMean        = 80.0;   // mean ADC counts for a single photon
      double speError       = 16.0;   // sigma of ADC counts for a single photon
      double pedMean        = 200.0;  // mean ADC counts for the pedestal
      double pedError       = 3.0;    // sigma of ADC counts for the pedestal

      // SiPM pixels
      bool   enablePixelGaps = false; // enable/disable removal of hits in gaps between pixels
      double pixelSize       = 3.0;   // pixel (active) size // [mm]

      // overall safety factor
      /* simulations assume the detector is ideal and perfect, but reality is
       * often neither; use this safety factor to reduce the number of initial
       * photon hits for a more conservative estimate of the number of
       * photoelectrons, or set to 1 to apply no such factor
       */
      double safetyFactor = 1.0; // allowed range: (0,1]

      // quantum efficiency
      // - wavelength units are [nm]
      // FIXME: figure out how users can override this, maybe an external `yaml` file
      std::vector<std::pair<double, double> > quantumEfficiency = {
        {325, 0.04},
        {340, 0.10},
        {350, 0.20},
        {370, 0.30},
        {400, 0.35},
        {450, 0.40},
        {500, 0.38},
        {550, 0.35},
        {600, 0.27},
        {650, 0.20},
        {700, 0.15},
        {750, 0.12},
        {800, 0.08},
        {850, 0.06},
        {900, 0.04}
      };
      /*
         std::vector<std::pair<double, double> > quantumEfficiency = { // test unit QE
         {325, 1.00},
         {900, 1.00}
         };
         */

      // ------------------------------------------------

      // print all parameters
      void Print(std::shared_ptr<spdlog::logger> m_log, spdlog::level::level_enum lvl=spdlog::level::debug) {
        m_log->log(lvl, "{:=^60}"," PhotoMultiplierHitDigiConfig Settings ");
        auto print_param = [&m_log, &lvl] (auto name, auto val) {
          m_log->log(lvl, "  {:>20} = {:<}", name, val);
        };
        print_param("seed",seed);
        print_param("hitTimeWindow",hitTimeWindow);
        print_param("timeResolution",timeResolution);
        print_param("speMean",speMean);
        print_param("speError",speError);
        print_param("pedMean",pedMean);
        print_param("pedError",pedError);
        print_param("enablePixelGaps",enablePixelGaps);
        print_param("pixelSize",pixelSize);
        print_param("safetyFactor",safetyFactor);
        m_log->log(lvl, "{:-^60}"," Quantum Efficiency vs. Wavelength ");
        for(auto& [wl,qe] : quantumEfficiency)
          m_log->log(lvl, "  {:>10} {:<}",wl,qe);
      }

  };
}
