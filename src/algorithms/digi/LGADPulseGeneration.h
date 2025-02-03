// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#pragma once

#include <DDRec/CellIDPositionConverter.h>
#include <algorithms/algorithm.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/digi/LGADPulseGenerationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using LGADPulseGenerationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<edm4hep::RawTimeSeriesCollection>>;

class LGADPulseGeneration : public LGADPulseGenerationAlgorithm,
                            public WithPodConfig<LGADPulseGenerationConfig> {
// The key pair is <NO of 40 MHz cycle since t = 0, cellID>
typedef std::unordered_map<dd4hep::rec::CellID, std::unordered_map<int, std::vector<double>>> AdcArray;

public:
  LGADPulseGeneration(std::string_view name)
      : LGADPulseGenerationAlgorithm{name, {"RawHits"}, {"OutputPulses"}, {}} {}
  virtual void init() final;
  void process(const Input&, const Output&) const final;

private:
  class PulseShape {
  public:
    // return voltage in units of ADC value
    // ranges from 0 to adc_range
    // charge is in unit GeV, it's the EDep in the detector
    virtual double Eval(double time, double hit_time, double charge) = 0;
    virtual ~PulseShape() {};
  };

  // default pulse shape is Landau
  class LandauPulse : public PulseShape {
  public:
    LandauPulse(double gain, double Vm, double sigma_analog, double adc_range) {
      m_gain = gain;
      m_sigma_analog = sigma_analog;

      // calculation of the extreme values for Landau distribution can be found on lin 514-520 of
      // https://root.cern.ch/root/html524/src/TMath.cxx.html#fsokrB Landau reaches minimum for mpv =
      // 0 and sigma = 1 at x = -0.22278
      const double x_when_landau_min = -0.22278;
      const double landau_min    = -gain * TMath::Landau(x_when_landau_min, 0, 1, kTRUE) / m_sigma_analog;
      m_scalingFactor = 1. / Vm / landau_min * adc_range;
    }
    double Eval(double time, double hit_time, double charge) {
      return charge * m_gain * TMath::Landau(time, hit_time, m_sigma_analog, kTRUE) * m_scalingFactor;
    }
  private:
    double m_gain;
    double m_sigma_analog;
    double m_scalingFactor;
  };

  double _Landau(double amp, double x, double mean, double std) const;
  void _FillADCArray(AdcArray& adc_sum, double charge, double mpv_analog,
                  int n_EICROC_cycle, dd4hep::rec::CellID cellID) const;
  std::unique_ptr<PulseShape> m_pulse;
};

} // namespace eicrecon
