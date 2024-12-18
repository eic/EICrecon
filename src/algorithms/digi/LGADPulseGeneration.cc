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
#include "LGADPulseGeneration.h"
#include "algorithms/digi/LGADPulseGenerationConfig.h"

namespace eicrecon {

LandauPulse::LandauPulse(double gain, double Vm, double sigma_analog, double adc_range) {
  m_gain = gain;
  m_sigma_analog = sigma_analog;

  // calculation of the extreme values for Landau distribution can be found on lin 514-520 of
  // https://root.cern.ch/root/html524/src/TMath.cxx.html#fsokrB Landau reaches minimum for mpv =
  // 0 and sigma = 1 at x = -0.22278
  const double x_when_landau_min = -0.22278;
  const double landau_min    = -gain * TMath::Landau(x_when_landau_min, 0, 1, kTRUE) / m_sigma_analog;
  m_scalingFactor = 1. / Vm / landau_min * adc_range;
}

double LandauPulse::Eval(double time, double hit_time, double charge) {
  return charge * m_gain * TMath::Landau(time, hit_time, m_sigma_analog, kTRUE) * m_scalingFactor;
}

void LGADPulseGeneration::_FillADCArray(AdcArray& adc_sum, double charge, double mpv_analog, int n_EICROC_cycle, dd4hep::rec::CellID cellID) const {
  double Vm       = m_cfg.Vm;
  double t        = 0;
  double tMax     = m_cfg.tMax;
  double interval = m_cfg.tInterval;
  int adc_range   = m_cfg.adc_range;
  int nBins       = m_cfg.tdc_range;

  // amplitude has to be negative
  // because voltage is negative
  // fetch the corresponding array
  auto& ADCs = adc_sum[cellID][n_EICROC_cycle];
  if (ADCs.size() == 0)
    ADCs.resize(nBins, 0);

  // keep filling the array until added amplitude < ignore_thres
  for (unsigned int j = 0; j <= ADCs.size(); ++j, t += interval) {
    double amplitude = m_pulse -> Eval(t, mpv_analog, charge);
    if(std::fabs(amplitude) > std::fabs(m_cfg.ignore_thres * adc_range/m_cfg.Vm)){
      if(j >= ADCs.size()) {
        // pulse has to be saved in the next clock cycle
        this -> _FillADCArray(adc_sum, charge, mpv_analog - tMax, n_EICROC_cycle+1, cellID);
      } else ADCs[j] += amplitude;
    }
  }

}

void LGADPulseGeneration::process(const LGADPulseGeneration::Input& input,
                                 const LGADPulseGeneration::Output& output) const {
  const auto [simhits] = input;
  auto [rawADCs]       = output;

  int tdc_range   = m_cfg.tdc_range;

  // signal sum
  // NOTE: we take the cellID of the most energetic hit in this group so it is a real cellID from an
  // MC hit
  AdcArray adc_sum;

  for (const auto& hit : *simhits) {
    auto cellID       = hit.getCellID();

    double time   = hit.getTime() * dd4hep::ns;
    double charge = hit.getEDep();
    // reduce computation power by not simulating low-charge hits
    if (charge < m_cfg.ignore_thres)
      continue;

    int n_EICROC_cycle = int(time/m_cfg.tMax + 1e-3);
    double time_in_cycle = time - n_EICROC_cycle * m_cfg.tMax;
    double mpv_analog = time_in_cycle + m_cfg.risetime;
    this -> _FillADCArray(adc_sum, charge, mpv_analog, n_EICROC_cycle, cellID);
  }

  // convert vector of ADC values to RawTimeSeries
  for (const auto& [cellID, nCycleADCs] : adc_sum) {
    for (const auto& [nCycle, ADCs] : nCycleADCs) {
      auto time_series = rawADCs->create();
      time_series.setCellID(cellID);
      time_series.setTime(nCycle * m_cfg.tMax);
      time_series.setCharge(1.); // placeholder. Don't know what to assign when there are two or more hits
      time_series.setInterval(m_cfg.tInterval);

      for (const auto ADC : ADCs)
        time_series.addToAdcCounts(ADC);
    }
  }
} // LGADPulseGeneration:process
} // namespace eicrecon
