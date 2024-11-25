// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#include <DDRec/CellIDPositionConverter.h>
#include <Evaluator/DD4hepUnits.h>
#include <RtypesCore.h>
#include <gsl/pointers>
#include <unordered_map>
#include <vector>

#include "TMath.h"
#include "TOFPulseGeneration.h"
#include "algorithms/digi/TOFHitDigiConfig.h"


namespace eicrecon {

double TOFPulseGeneration::_Landau(double amp, double x, double mean, double std) const {
  return amp*TMath::Landau(x, mean, std, kTRUE);
}

double TOFPulseGeneration::_DigitizeTime(double time) const {
  // digitization always round down to the previous bin
  return static_cast<int>(time/m_cfg.tInterval + 1e-3)*m_cfg.tInterval;
}

void TOFPulseGeneration::process(const TOFPulseGeneration::Input& input,
                                 const TOFPulseGeneration::Output& output) const {
  const auto [simhits] = input;
  auto [rawADCs] = output;

  double interval = m_cfg.tInterval;
  double Vm = m_cfg.Vm;
  double tMin = _DigitizeTime(m_cfg.tMin);
  double tMax = _DigitizeTime(m_cfg.tMax);
  int adc_range = m_cfg.adc_range;
  int tdc_range = m_cfg.tdc_range;
  int nBins = m_cfg.tdc_range;

  // signal sum
  // NOTE: we take the cellID of the most energetic hit in this group so it is a real cellID from an
  // MC hit
  std::unordered_map<dd4hep::rec::CellID, std::vector<double>> adc_sum;

  for (const auto& hit : *simhits) {
    auto cellID          = hit.getCellID();
    double sum_charge = 0.0;
    double mpv_analog = 0.0;

    double  time       = hit.getTime() * dd4hep::ns;
    double  charge     = hit.getEDep();
    // reduce computation power by not simulating low-charge hits
    if(charge < m_cfg.ignore_thres) continue;

    auto& ADCs = adc_sum[cellID];
    if(ADCs.size() == 0) ADCs.resize(nBins, 0);

    mpv_analog = time + m_cfg.risetime;

    // amplitude has to be negative
    // because voltage is negative
    // calculation of the extreme values for Landau distribution can be found on lin 514-520 of https://root.cern.ch/root/html524/src/TMath.cxx.html#fsokrB
    // Landau reaches minimum for mpv = 0 and sigma = 1 at x = -0.22278
    const double x_when_landau_min = -0.22278;
    double landau_min = this -> _Landau(-m_cfg.gain, x_when_landau_min, 0, 1)/m_cfg.sigma_analog;
    double scalingFactor = 1. / Vm / landau_min * adc_range;

    {
        int j;
        for (double t = tMin, j = 0; t < tMax; ++j, t += interval)
          ADCs[j] += charge * this -> _Landau(-m_cfg.gain, t, mpv_analog, m_cfg.sigma_analog) * scalingFactor;
    }

  }

 // convert vector of ADC values to RawTimeSeries
 for(const auto &[cellID, ADCs] : adc_sum) {
    auto time_series = rawADCs -> create();
    time_series.setCellID(cellID);
    time_series.setTime(tMin);
    time_series.setCharge(1.); // placeholder. Don't know what to assign when there are two or more hits
    time_series.setInterval(interval);

    for(const auto ADC : ADCs)
      time_series.addToAdcCounts(ADC);
  }
} // TOFPulseGeneration:process
} // namespace eicrecon
