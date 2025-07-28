// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy
//
// Convert energy deposition into analog pulses

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/SimTrackerHitCollection.h>
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
#include <edm4eic/SimPulseCollection.h>
#else
#include <edm4hep/TimeSeriesCollection.h>
#endif
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "algorithms/digi/SiliconPulseGenerationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
using PulseType = edm4eic::SimPulse;
#else
using PulseType = edm4hep::TimeSeries;
#endif

using SiliconPulseGenerationAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<PulseType::collection_type>>;

class SignalPulse {

public:
  virtual ~SignalPulse()                                = default; // Virtual destructor
  virtual double operator()(double time, double charge) const = 0;
  virtual double getMaximumTime() const                 = 0;
};

// Landau Pulse Shape Functor
class LandauPulse : public SignalPulse {

public:
  LandauPulse(std::vector<double> params);
  double operator()(double time, double charge) const override;
  double getMaximumTime() const override;

private:
  double m_gain             = 1.0;
  double m_sigma_analog     = 1.0;
  double m_hit_sigma_offset = 3.5;
};

// EvaluatorSvc Pulse
class EvaluatorPulse : public SignalPulse {

public:
  EvaluatorPulse(const std::string& expression, const std::vector<double>& params);
  double operator()(double time, double charge) const override;
  double getMaximumTime() const override;

private:
  std::unordered_map<std::string, double> param_map;
  std::function<double(const std::unordered_map<std::string, double>&)> m_evaluator;
};

class PulseShapeFactory {

public:
  static std::unique_ptr<SignalPulse> createPulseShape(const std::string& type,
                                                       const std::vector<double>& params);
};

class SiliconPulseGeneration : public SiliconPulseGenerationAlgorithm,
                               public WithPodConfig<SiliconPulseGenerationConfig> {

public:
  SiliconPulseGeneration(std::string_view name)
      : SiliconPulseGenerationAlgorithm{name, {"RawHits"}, {"OutputPulses"}, {}} {}
  void init() final;
  void process(const Input&, const Output&) const final;

private:
  std::shared_ptr<SignalPulse> m_pulse;
  float m_min_sampling_time = 0 * edm4eic::unit::ns;
};

} // namespace eicrecon
