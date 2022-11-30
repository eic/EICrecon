#pragma once

#include <Evaluator/DD4hepUnits.h>

namespace eicrecon {
  struct PhotoMultiplierHitDigiConfig {

    double hitTimeWindow = 20.0*dd4hep::ns;
    double timeStep      = 0.0625*dd4hep::ns;
    double speMean       = 80.0;
    double speError      = 16.0;
    double pedMean       = 200.0;
    double pedError      = 3.0;
    
    // FIXME: figure out how users can override this, maybe an external `yaml` file
    /*
    std::vector<std::pair<double, double> > quantumEfficiency = { // test unit QE
      {325*dd4hep::nm, 1.00},
      {900*dd4hep::nm, 1.00}
    };
    */
    std::vector<std::pair<double, double> > quantumEfficiency = {
      {325*dd4hep::nm, 0.04},
      {340*dd4hep::nm, 0.10},
      {350*dd4hep::nm, 0.20},
      {370*dd4hep::nm, 0.30},
      {400*dd4hep::nm, 0.35},
      {450*dd4hep::nm, 0.40},
      {500*dd4hep::nm, 0.38},
      {550*dd4hep::nm, 0.35},
      {600*dd4hep::nm, 0.27},
      {650*dd4hep::nm, 0.20},
      {700*dd4hep::nm, 0.15},
      {750*dd4hep::nm, 0.12},
      {800*dd4hep::nm, 0.08},
      {850*dd4hep::nm, 0.06},
      {900*dd4hep::nm, 0.04}
    };

  };
}
