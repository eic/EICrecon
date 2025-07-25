// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

#include "SiliconPulseGeneration.h"

#include <RtypesCore.h>
#include <TMath.h>
#include <algorithms/service.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <gsl/pointers>
#include <stdexcept>
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

void SiliconPulseGeneration::init() {
  m_pulse =
      PulseShapeFactory::createPulseShape(m_cfg.pulse_shape_function, m_cfg.pulse_shape_params);
  m_min_sampling_time = m_cfg.min_sampling_time;

  if (m_pulse->getMaximumTime() > m_min_sampling_time) {
    m_min_sampling_time = m_pulse->getMaximumTime();
  }
}

void SiliconPulseGeneration::process(const SiliconPulseGeneration::Input& input,
                                     const SiliconPulseGeneration::Output& output) const {
  const auto [simhits] = input;
  auto [rawPulses]     = output;

  for (const auto& hit : *simhits) {

    auto cellID   = hit.getCellID();
    double time   = hit.getTime();
    double charge = hit.getEDep();

    // Calculate nearest timestep to the hit time rounded down (assume clocks aligned with time 0)
    double signal_time = m_cfg.timestep * std::floor(time / m_cfg.timestep);

    bool passed_threshold   = false;
    std::uint32_t skip_bins = 0;
    float integral          = 0;
    std::vector<float> pulse;

    for (std::uint32_t i = 0; i < m_cfg.max_time_bins; i++) {
      double t    = signal_time + i * m_cfg.timestep - time;
      auto signal = (*m_pulse)(t, charge);
      if (std::abs(signal) < m_cfg.ignore_thres) {
        if (!passed_threshold) {
          skip_bins = i;
          continue;
        }
        if (t > m_min_sampling_time) {
          break;
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
    time_series.setCellID(cellID);
    time_series.setInterval(m_cfg.timestep);
    time_series.setTime(signal_time + skip_bins * m_cfg.timestep);

    for (const auto& value : pulse) {
      time_series.addToAmplitude(value);
    }

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
    time_series.setIntegral(integral);
    time_series.setPosition(
        edm4hep::Vector3f(hit.getPosition().x, hit.getPosition().y, hit.getPosition().z));
    time_series.addToTrackerHits(hit);
    time_series.addToParticles(hit.getParticle());
#endif
  }

} // SiliconPulseGeneration:process

} // namespace eicrecon
