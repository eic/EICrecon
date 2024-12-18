// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul, Chun Yuen Tsang, Prithwish Tribedy
// Special Acknowledgement: Kolja Kauder
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#pragma once

#include <algorithms/algorithm.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <string>
#include <string_view>

#include "algorithms/digi/LGADPulseGenerationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using LGADPulseGenerationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<edm4hep::RawTimeSeriesCollection>>;

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
  LandauPulse(double gain, double Vm, double sigma_analog, double adc_range);
  double Eval(double time, double hit_time, double charge);
private:
  double m_gain;
  double m_sigma_analog;
  double m_scalingFactor;
};


class LGADPulseGeneration : public LGADPulseGenerationAlgorithm,
                           public WithPodConfig<LGADPulseGenerationConfig> {
// The key pair is <NO of 40 MHz cycle since t = 0, cellID>
typedef std::unordered_map<dd4hep::rec::CellID, std::unordered_map<int, std::vector<double>>> AdcArray;

public:
  LGADPulseGeneration(std::string_view name, std::unique_ptr<PulseShape>&& pulse)
      : LGADPulseGenerationAlgorithm{name, {"RawHits"}, {"OutputPulses"}, {}}, m_pulse(std::move(pulse)) {}
  virtual void init(){};
  void process(const Input&, const Output&) const final;

protected:
  double _Landau(double amp, double x, double mean, double std) const;
  void _FillADCArray(AdcArray& adc_sum, double charge, double mpv_analog,
                  int n_EICROC_cycle, dd4hep::rec::CellID cellID) const;
  std::unique_ptr<PulseShape> m_pulse;
};

} // namespace eicrecon
