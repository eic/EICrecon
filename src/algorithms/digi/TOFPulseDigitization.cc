// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert ADC pulses from TOFPulseGeneration into ADC and TDC values
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

#include "TOFPulseDigitization.h"
#include "Math/SpecFunc.h"
#include "algorithms/digi/TOFHitDigiConfig.h"

// using namespace dd4hep;
// using namespace dd4hep::Geometry;
// using namespace dd4hep::DDRec;
// using namespace eicrecon;
using namespace dd4hep::xml;

namespace eicrecon {

void TOFPulseDigitization::process(const TOFPulseDigitization::Input& input,
                                   const TOFPulseDigitization::Output& output) const {
  const auto [simhits] = input;
  auto [rawhits] = output;

  double thres = m_cfg.t_thres;
  // double Vm=-0.05;
  //  SP noted that max dE experienced by LGAD should be 0.8 keV
  double Vm = m_cfg.Vm;
  int adc_range = m_cfg.adc_range;

  // normalized time threshold
  // convert threshold EDep to voltage
  double norm_threshold = -thres * adc_range / Vm;

  for(const auto& pulse : *simhits) {
    // Added by SP
    //-------------------------------------------------------------
    double intersectionX = 0.0;
    int tdc              = std::numeric_limits<int>::max();
    int adc              = 0;
    double V             = 0.0;

    int time_bin = 0;
    double adc_prev = 0;
    double time_interval = pulse.getInterval();
    auto adcs = pulse.getAdcCounts();
    for (const auto adc : adcs) {
      if (adc_prev >= norm_threshold && adc <= norm_threshold) {
        intersectionX = time_bin*time_interval + time_interval * (norm_threshold - adc_prev) / (adc - adc_prev);
        tdc = ceil(intersectionX / 0.02);
      }
      if (abs(adc) > abs(V)) // To get peak of the Analog signal
        V = adc;
      adc_prev = adc;
      ++time_bin;
    }

    // limit the range of adc values
    adc = std::min(static_cast<double>(adc_range), round(-V));
    // only store valid hits
    if (tdc < std::numeric_limits<int>::max())
      rawhits->create(pulse.getCellID(), adc, tdc);
    //-----------------------------------------------------------

  }
} // TOFPulseDigitization:process
} // namespace eicrecon
