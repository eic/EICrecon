// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder

// A general digitization for CalorimeterHit from simulation
// 1. Smear energy deposit with a/sqrt(E/GeV) + b + c/E or a/sqrt(E/GeV) (relative value)
// 2. Digitize the energy with dynamic ADC range and add pedestal (mean +- sigma)
// 3. Time conversion with smearing resolution (absolute value)
// 4. Signal is summed if the SumFields are provided
//
// Author: Souvik Paul, Chun Yuen Tsang
// Date: 19/07/2024

#pragma once

#include "TF1.h"
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <spdlog/spdlog.h>

#include "DD4hep/Detector.h"
#include "DDRec/Surface.h"
#include <DDRec/CellIDPositionConverter.h>

#include "BarrelTOFNeighborFinder.h"
#include "algorithms/digi/BTOFHitDigiConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

class BTOFHitDigi : public WithPodConfig<BTOFHitDigiConfig> {

public:
  BTOFHitDigi()
      : fLandau(
            "landau",
            [](Double_t* x, Double_t* par) {
              Double_t mean = par[0]; // Mean
              Double_t std  = par[1]; // Standard deviation
              Double_t C    = -113.766;

              Double_t landau = C * TMath::Landau((x[0]), mean, std, kTRUE);

              return landau;
            },
            tMin, tMax, 2) {}

  void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger);

  std::unique_ptr<edm4eic::RawTrackerHitCollection>
  execute(const edm4hep::SimTrackerHitCollection* simhits);
  std::vector<bool> ToDigitalCode(int value, int numBits);

protected:
  BarrelTOFNeighborFinder _neighborFinder;
  std::shared_ptr<spdlog::logger> m_log;
  double _integralGaus(double mean, double sd, double low_lim, double up_lim);

  // unitless counterparts of inputs
  double dyRangeADC{0}, stepTDC{0}, tRes{0};
  const double pi       = TMath::Pi();
  const double tMin     = 0.1;
  const double tMax     = 100.0;
  const int total_time  = ceil(tMax - tMin);
  const int time_period = 25;
  const int nBins       = 10000;
  const int adc_bit     = 8;
  const int tdc_bit     = 10;

  int adc_range;
  int tdc_range;

  uint64_t id_mask{0};
  TF1 fLandau;

  std::default_random_engine generator;        // TODO: need something more appropriate here
  std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1
};

} // namespace eicrecon
