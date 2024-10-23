// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function
//
// Author: Souvik Paul, Chun Yuen Tsang
// Date: 22/10/2024

#include "DD4hep/Detector.h"
#include "DDRec/Surface.h"
#include "TF1.h"
#include "TMath.h"
#include <Evaluator/DD4hepUnits.h>
#include <TGraph.h>
#include <bitset>
#include <fmt/format.h>
#include <iostream>
#include <vector>

#include "TOFPulseGeneration.h"
#include "Math/SpecFunc.h"
#include "algorithms/digi/TOFHitDigiConfig.h"

// using namespace dd4hep;
// using namespace dd4hep::Geometry;
// using namespace dd4hep::DDRec;
// using namespace eicrecon;
using namespace dd4hep::xml;

namespace eicrecon {

double TOFPulseGeneration::_Landau(double x, double mean, double std) const {
  double C = -113.755;
  return C * TMath::Landau(x, mean, std, kTRUE);
}

void TOFPulseGeneration::process(const TOFPulseGeneration::Input& input,
                                 const TOFPulseGeneration::Output& output) const {
  const auto [simhits] = input;
  auto [rawADCs] = output;

  //  SP noted that max dE experienced by LGAD should be 0.8 keV
  double Vm = m_cfg.Vm;
  double tMin = m_cfg.tMin;
  double tMax = m_cfg.tMax;
  int adc_range = m_cfg.adc_range;
  int tdc_range = m_cfg.tdc_range;
  int nBins = m_cfg.nBins;

  // signal sum
  // NOTE: we take the cellID of the most energetic hit in this group so it is a real cellID from an
  // MC hit
  std::unordered_map<dd4hep::rec::CellID, std::vector<double>> adc_sum;
  double interval = (tMax - tMin) / (nBins - 1);

  for (const auto& hit : *simhits) {
    auto cellID          = hit.getCellID();
    double sum_charge = 0.0;
    double mpv_analog = 0.0; // SP

    double  time       = hit.getTime();
    double  charge     = hit.getEDep() * m_cfg.gain;
    // reduce computation power by not simulating low-charge hits
    if(charge < m_cfg.ignore_thres) continue;

    auto& ADCs = adc_sum[cellID];
    if(ADCs.size() == 0) ADCs.resize(nBins, 0);

    // Added by SP
    //-------------------------------------------------------------
    mpv_analog = time + m_cfg.risetime;

    double landau_min = this -> _Landau(0, mpv_analog, m_cfg.sigma_analog);
    // find minimum of the landau function
    // minimum = peak because prefactor is negative
    for (int j = 0; j < nBins; ++j) {
      double x = tMin + j * interval;
      landau_min = std::min(landau_min, this -> _Landau(x, mpv_analog, m_cfg.sigma_analog));
    }

    double scalingFactor = -1. / Vm / landau_min * adc_range;

    for (int j = 0; j < nBins; ++j) {
      double x = tMin + j * interval;
      double y = -1 * charge * this -> _Landau(x, mpv_analog, m_cfg.sigma_analog) * scalingFactor;
      ADCs[j] += y;;
    }

  }

 // convert vector of ADC values to RawTimeSeries
 for(const auto &[cellID, ADCs] : adc_sum) {
    auto time_series = rawADCs -> create();
    time_series.setCellID(cellID);
    time_series.setTime(1.); // placeholder. Don't know what to assign when there are two or more hits
    time_series.setCharge(1.); // placeholder. Don't know what to assign when there are two or more hits
    time_series.setInterval(interval);

    for(const auto ADC : ADCs)
      time_series.addToAdcCounts(ADC);
  }
} // TOFPulseGeneration:process
} // namespace eicrecon
