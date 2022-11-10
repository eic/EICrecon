#pragma once

namespace eicrecon {
  struct PhotoMultiplierHitDigiConfig {
    std::vector<std::pair<double, double> > quantumEfficiency; // {{2.6*eV, 0.3}, {7.0*eV, 0.3}}
    double hitTimeWindow; // 20.0*ns
    double timeStep;      // 0.0625*ns
    double speMean;       // 80.0
    double speError;      // 16.0
    double pedMean;       // 200.0
    double pedError;      // 3.0
  };
}
