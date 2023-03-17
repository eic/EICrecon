#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {
  class PhotoMultiplierHitDigiConfig {
    public:

      // random number generator seed
      unsigned long seed = 37;

      // triggering
      double hitTimeWindow = 20.0;   // [ns]
      double timeStep      = 0.0625; // [ns]
      double speMean       = 80.0;
      double speError      = 16.0;
      double pedMean       = 200.0;
      double pedError      = 3.0;

      // SiPM pixels
      bool   enablePixelGaps = false; // enable/disable removal of hits in gaps between pixels
      double pixelSize       = 3.0;   // [mm] // pixel (active) size

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

      // noise
      bool noiseInjection = false;
      double noiseRate = 20000; // [Hz]
      double timeWindow = 20.;  // [ns]

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
        auto puts = [&m_log, &lvl] (auto name, auto val) {
          m_log->log(lvl, "  {:>20} = {:<}", name, val);
        };
        puts("seed",seed);
        puts("hitTimeWindow",hitTimeWindow);
        puts("timeStep",timeStep);
        puts("speMean",speMean);
        puts("speError",speError);
        puts("pedMean",pedMean);
        puts("pedError",pedError);
        puts("enablePixelGaps",enablePixelGaps);
        puts("pixelSize",pixelSize);
        puts("safetyFactor",safetyFactor);
        puts("noiseRate",noiseRate);
        puts("timeWindow",timeWindow);
        m_log->log(lvl, "{:-^60}"," Quantum Efficiency vs. Wavelength ");
        for(auto& [wl,qe] : quantumEfficiency)
          m_log->log(lvl, "  {:>10} {:<}",wl,qe);
      }

  };
}
