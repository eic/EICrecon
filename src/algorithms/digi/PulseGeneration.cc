// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy,
//                         Minho Kim, Sylvester Joosten, Wouter Deconinck, Dmitry Kalinkin
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#include <edm4eic/EDM4eicVersion.h>
#include "PulseGeneration.h"

#include <RtypesCore.h>
#include <TMath.h>
#include <algorithms/service.h>
#include <edm4hep/CaloHitContribution.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3f.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "services/evaluator/EvaluatorSvc.h"

namespace eicrecon {

class SignalPulse {

public:
  virtual ~SignalPulse() = default; // Virtual destructor

  virtual double operator()(double time, double charge) = 0;

  virtual double getMaximumTime() const = 0;
};

// ----------------------------------------------------------------------------
// Landau Pulse Shape Functor
// ----------------------------------------------------------------------------
class LandauPulse : public SignalPulse {
public:
  LandauPulse(std::vector<double> params) {

    if ((params.size() != 2) && (params.size() != 3)) {
      throw std::runtime_error(
          "LandauPulse requires 2 or 3 parameters, gain, sigma_analog, [hit_sigma_offset], got " +
          std::to_string(params.size()));
    }

    m_gain         = params[0];
    m_sigma_analog = params[1];
    if (params.size() == 3) {
      m_hit_sigma_offset = params[2];
    }
  };

  double operator()(double time, double charge) override {
    return charge * m_gain *
           TMath::Landau(time, m_hit_sigma_offset * m_sigma_analog, m_sigma_analog, kTRUE);
  }

  double getMaximumTime() const override { return m_hit_sigma_offset * m_sigma_analog; }

private:
  double m_gain             = 1.0;
  double m_sigma_analog     = 1.0;
  double m_hit_sigma_offset = 3.5;
};

// EvaluatorSvc Pulse
class EvaluatorPulse : public SignalPulse {
public:
  EvaluatorPulse(const std::string& expression, const std::vector<double>& params) {

    std::vector<std::string> keys = {"time", "charge"};
    for (std::size_t i = 0; i < params.size(); i++) {
      std::string p = "param" + std::to_string(i);
      //Check the expression contains the parameter
      if (expression.find(p) == std::string::npos) {
        throw std::runtime_error("Parameter " + p + " not found in expression");
      }
      keys.push_back(p);
      param_map[p] = params[i];
    }

    // Check the expression is contains time and charge
    if (expression.find("time") == std::string::npos) {
      throw std::runtime_error("Parameter [time] not found in expression");
    }
    if (expression.find("charge") == std::string::npos) {
      throw std::runtime_error("Parameter [charge] not found in expression");
    }

    auto& serviceSvc = algorithms::ServiceSvc::instance();
    m_evaluator      = serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")->_compile(expression, keys);
  };

  double operator()(double time, double charge) override {
    param_map["time"]   = time;
    param_map["charge"] = charge;
    return m_evaluator(param_map);
  }

  double getMaximumTime() const override { return 0; }

private:
  std::unordered_map<std::string, double> param_map;
  std::function<double(const std::unordered_map<std::string, double>&)> m_evaluator;
};

class PulseShapeFactory {
public:
  static std::unique_ptr<SignalPulse> createPulseShape(const std::string& type,
                                                       const std::vector<double>& params) {
    if (type == "LandauPulse") {
      return std::make_unique<LandauPulse>(params);
    }
    //
    // Add more pulse shape variants here as needed

    // If type not found, try and make a function using the ElavulatorSvc
    try {
      return std::make_unique<EvaluatorPulse>(type, params);
    } catch (...) {
      throw std::invalid_argument("Unable to make pulse shape type: " + type);
    }
  }
};

std::tuple<double, double>
HitAdapter<edm4hep::SimTrackerHit>::getPulseSources(const edm4hep::SimTrackerHit& hit) {
  return {hit.getTime(), hit.getEDep()};
}

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
void HitAdapter<edm4hep::SimTrackerHit>::addRelations(MutablePulseType& pulse,
                                                      const edm4hep::SimTrackerHit& hit) {
  pulse.addToTrackerHits(hit);
  pulse.addToParticles(hit.getParticle());
}
#endif

std::tuple<double, double>
HitAdapter<edm4hep::SimCalorimeterHit>::getPulseSources(const edm4hep::SimCalorimeterHit& hit) {
  const auto& contribs  = hit.getContributions();
  auto earliest_contrib = std::ranges::min_element(
      contribs, [](const auto& a, const auto& b) { return a.getTime() < b.getTime(); });
  return {earliest_contrib->getTime(), hit.getEnergy()};
}

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
void HitAdapter<edm4hep::SimCalorimeterHit>::addRelations(MutablePulseType& pulse,
                                                          const edm4hep::SimCalorimeterHit& hit) {
  pulse.addToCalorimeterHits(hit);
  pulse.addToParticles(hit.getContributions(0).getParticle());
}
#endif

template <typename HitT> void PulseGeneration<HitT>::init() {
  m_pulse =
      PulseShapeFactory::createPulseShape(m_cfg.pulse_shape_function, m_cfg.pulse_shape_params);
  m_min_sampling_time = m_cfg.min_sampling_time;

  m_min_sampling_time = std::max<double>(m_pulse->getMaximumTime(), m_min_sampling_time);
}

template <typename HitT>
void PulseGeneration<HitT>::process(
    const typename PulseGenerationAlgorithm<HitT>::Input& input,
    const typename PulseGenerationAlgorithm<HitT>::Output& output) const {
  const auto [simhits] = input;
  auto [rawPulses]     = output;

  for (const auto& hit : *simhits) {
    const auto [time, charge] = HitAdapter<HitT>::getPulseSources(hit);

    // Calculate nearest timestep to the hit time rounded down (assume clocks aligned with time 0)
    double signal_time = m_cfg.timestep * std::floor(time / m_cfg.timestep);

    bool passed_threshold   = false;
    std::uint32_t skip_bins = 0;
    float previous          = 0;
    float integral          = 0;
    std::vector<float> pulse;

    for (std::uint32_t i = 0; i < m_cfg.max_time_bins; i++) {
      double t    = signal_time + i * m_cfg.timestep - time;
      auto signal = (*m_pulse)(t, charge);

      // Early exit conditions: below threshold and falling, or min sampling time after threshold
      if (std::abs(signal) < m_cfg.ignore_thres) {
        if (!passed_threshold) {
          // Before threshold crossed
          auto diff = std::abs(signal) - std::abs(previous);
          previous  = signal;
          if (diff >= 0) {
            // Rising before threshold crossed
            skip_bins = i;
            continue;
          } else {
            // Falling without threshold ever crossed
            break;
          }
        } else {
          // After threshold crossed, stop after min sampling time
          if (t > m_min_sampling_time) {
            break;
          }
        }
      }

      passed_threshold = true;
      pulse.push_back(signal);
      integral += signal;
    }

    if (!passed_threshold) {
      continue;
    }

    auto time_series = rawPulses->create();
    time_series.setCellID(hit.getCellID());
    time_series.setInterval(m_cfg.timestep);
    time_series.setTime(signal_time + skip_bins * m_cfg.timestep);

    for (const auto& value : pulse) {
      time_series.addToAmplitude(value);
    }

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
    time_series.setIntegral(integral);
    time_series.setPosition(
        edm4hep::Vector3f(hit.getPosition().x, hit.getPosition().y, hit.getPosition().z));
    HitAdapter<HitT>::addRelations(time_series, hit);
#endif
  }

} // PulseGeneration:process

template class PulseGeneration<edm4hep::SimTrackerHit>;
template class PulseGeneration<edm4hep::SimCalorimeterHit>;

} // namespace eicrecon
