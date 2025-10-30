// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy,
//                         Minho Kim, Sylvester Joosten, Wouter Deconinck, Dmitry Kalinkin
//
// Convert energy deposition into ADC pulses
// ADC pulses are assumed to follow the shape of landau function

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
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "services/evaluator/EvaluatorSvc.h"

namespace eicrecon {

class SignalPulse {

public:
  // State for streaming fast-path iteration (when cache resolution == timestep)
  struct StreamState {
    int base_index;
    double frac;      // constant interpolation fraction across iterations
    double v_floor_n; // normalized value at base_index
    double v_ceil_n;  // normalized value at base_index+1
  };

  virtual ~SignalPulse() = default; // Virtual destructor

  // Public capability query: caching is only valid for pulses linear in charge
  bool supportsCaching() const { return isLinearInCharge(); }
  // Whether caching has been enabled (resolution > 0)
  bool cachingEnabled() const { return m_cache_resolution > 0.0; }
  // Whether linear interpolation between cached integer samples is safe.
  // Default: true for smooth/continuous pulse shapes (e.g., LandauPulse).
  // Pulses with discontinuities should override to return false.
  virtual bool supportsLinearInterpolation() const { return true; }

  // Main interface - with caching and interpolation
  double operator()(double time, double charge) {
    // If caching is disabled, use direct evaluation
    if (m_cache_resolution <= 0.0) {
      return evaluate(time, charge);
    }

    // Use caching with linear interpolation
    // Normalize time to cache resolution
    double normalized_time = time / m_cache_resolution;
    int index              = static_cast<int>(std::floor(normalized_time));

    // Get or compute cached values at floor and ceil indices
    double value_floor = getCachedValue(index, charge);
    double value_ceil  = getCachedValue(index + 1, charge);

    // Linear interpolation
    double fraction = normalized_time - index;
    return value_floor + fraction * (value_ceil - value_floor);
  }

  virtual double getMaximumTime() const = 0;

  // Enable caching with specified resolution (typically the timestep)
  void enableCache(double resolution) {
    // Validate linearity first to avoid allocating cache when unsupported
    validateLinearity();

    m_cache_resolution = resolution;
    // Pre-size array-backed cache and reset filled flags
    const std::size_t size = 2 * MAX_CACHE_SPAN_BINS + 3; // allow indices in [-MAX, MAX+2]
    m_cache_values.assign(size, std::numeric_limits<double>::quiet_NaN());
    m_cache_size   = size;
    m_cache_offset = static_cast<int>(MAX_CACHE_SPAN_BINS + 1);
  }

  // Expert fast-path: fetch normalized cached sample at integer time index.
  // Returns value for charge=1; callers can scale by actual charge.
  // Falls back to direct evaluate if index is outside the preallocated span.
  double sampleNormalizedAtIndex(int time_index) const {
    const int vec_idx = time_index + m_cache_offset;
    if (vec_idx < 0 || static_cast<std::size_t>(vec_idx) >= m_cache_size) {
      return evaluate(time_index * m_cache_resolution, 1.0);
    }
    double& slot = m_cache_values[static_cast<std::size_t>(vec_idx)];
    if (std::isnan(slot)) {
      const double time = time_index * m_cache_resolution;
      slot              = evaluate(time, 1.0);
    }
    return slot;
  }

  // Fetch normalized sample at an arbitrary (possibly fractional) time index.
  // Uses exact evaluate() for fractional indices to avoid interpolation artifacts
  // (important for discontinuous shapes, e.g., square pulses). For integral
  // indices, reuses the integer-index cache for performance.
  double sampleNormalizedAt(double time_index) const {
    // If caching is disabled, evaluate directly using absolute time
    if (m_cache_resolution <= 0.0) {
      return evaluate(time_index, 1.0);
    }
    const double rounded = std::round(time_index);
    if (std::fabs(time_index - rounded) < 1e-12) {
      return sampleNormalizedAtIndex(static_cast<int>(rounded));
    }
    return evaluate(time_index * m_cache_resolution, 1.0);
  }

  // Prepare streaming state for a pulse starting at signal_time with hit at time
  // Assumes cache resolution equals the timestep for O(1) index streaming.
  StreamState prepareStreaming(double signal_time, double time, double timestep) const {
    // We sample the pulse at t_rel(i) = (i - nt0), i=0..N-1, where nt0 = (signal_time - time)/dt.
    // Let s = -nt0; for i=0, t_rel = s. Each subsequent iteration advances by +1.
    const double nt0       = (signal_time - time) / timestep;
    const double s         = -nt0;
    const int base_index   = static_cast<int>(std::floor(s));
    const double frac      = s - base_index; // constant across iterations
    const double v_floor_n = sampleNormalizedAtIndex(base_index);
    const double v_ceil_n  = sampleNormalizedAtIndex(base_index + 1);
    return {.base_index=base_index, .frac=frac, .v_floor_n=v_floor_n, .v_ceil_n=v_ceil_n};
  }

protected:
  // Derived classes implement the actual pulse evaluation
  // IMPORTANT: evaluate() MUST be linear in charge for caching to work correctly!
  // That is: evaluate(t, a*q) must equal a * evaluate(t, q) for all t, q, a
  // The cache stores normalized values (charge=1) and scales by actual charge.
  virtual double evaluate(double time, double charge) const = 0;

  // Override this in derived classes if the pulse is NOT linear in charge
  // Default assumes linearity (which is true for LandauPulse and most physical pulses)
  virtual bool isLinearInCharge() const { return true; }

private:
  double m_cache_resolution = 0.0; // 0 means caching disabled
  // Set a maximum cache span of +/- MAX_CACHE_SPAN_BINS time indices, centered on 0.
  // This eliminates hash lookups by using O(1) array indexing.
  static constexpr std::size_t MAX_CACHE_SPAN_BINS = 10000;
  mutable std::vector<double> m_cache_values; // normalized values for charge=1
  std::size_t m_cache_size = 0;               // cached to avoid size() calls in hot path
  int m_cache_offset       = 0;               // vector index = time_index + m_cache_offset

  // Validate that the pulse function is linear in charge
  void validateLinearity() const {
    if (!isLinearInCharge()) {
      throw std::runtime_error(
          "SignalPulse caching was requested, but this pulse reports isLinearInCharge()==false. "
          "Caching only supports pulses that are linear in charge. Avoid calling enableCache() for "
          "non-linear pulses, or override isLinearInCharge() to return true if appropriate.");
    }
    // Runtime verification
    const double t_test = 1.0;
    const double q1 = 1.0;
    const double q2 = 2.0;
    const double v1 = evaluate(t_test, q1);
    const double v2 = evaluate(t_test, q2);
    const double ratio          = std::abs(v2 / v1);
    const double expected_ratio = q2 / q1;
    if (std::abs(ratio - expected_ratio) > 0.01 * expected_ratio) {
      throw std::runtime_error("SignalPulse caching linearity check FAILED: the pulse reported "
                               "linear (isLinearInCharge()==true) "
                               "but evaluate(t, a*q) != a * evaluate(t, q). Fix evaluate() to be "
                               "linear in charge or disable caching.");
    }
  }

  // Get cached value or compute and cache it
  double getCachedValue(int time_index, double charge) const {
    // Fast O(1) array-backed cache lookup. We store values for charge=1 and scale by 'charge'.
    const int vec_idx = time_index + m_cache_offset;
    if (vec_idx < 0 || static_cast<std::size_t>(vec_idx) >= m_cache_size) {
      // Outside the preallocated cache span: fall back to direct evaluation (no caching).
      return evaluate(time_index * m_cache_resolution, charge);
    }
    double& slot = m_cache_values[static_cast<std::size_t>(vec_idx)];
    if (std::isnan(slot)) {
      // Compute and cache normalized value (charge=1)
      const double time = time_index * m_cache_resolution;
      slot              = evaluate(time, 1.0);
    }
    return slot * charge;
  }
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

  double getMaximumTime() const override { return m_hit_sigma_offset * m_sigma_analog; }

protected:
  // LandauPulse is linear in charge: evaluate(t, a*q) = a*q*gain*Landau(...) = a*evaluate(t, q)
  double evaluate(double time, double charge) const override {
    return charge * m_gain *
           TMath::Landau(time, m_hit_sigma_offset * m_sigma_analog, m_sigma_analog, kTRUE);
  }

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
      // Check the expression contains the parameter
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

  double getMaximumTime() const override { return 0; }

  // Evaluator expressions can be discontinuous; avoid linear interpolation between cached points
  bool supportsLinearInterpolation() const override { return false; }

protected:
  // By default, consider evaluator-defined pulses as not guaranteed linear in charge
  // to avoid enabling caching unless explicitly overridden by a specialized subclass.
  bool isLinearInCharge() const override { return false; }

protected:
  // EvaluatorPulse: Linearity depends on the expression provided by user
  // Most physical pulse shapes multiply by charge, making them linear
  double evaluate(double time, double charge) const override {
    // Note: We need to modify param_map, but it doesn't change the logical state
    // of the pulse shape (same input -> same output), so this remains conceptually const
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    auto& mutable_map     = const_cast<std::unordered_map<std::string, double>&>(param_map);
    mutable_map["time"]   = time;
    mutable_map["charge"] = charge;
    return m_evaluator(param_map);
  }

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
  // Factory returns unique_ptr; construct shared_ptr to keep ownership simple across TUs
  auto uptr =
      PulseShapeFactory::createPulseShape(m_cfg.pulse_shape_function, m_cfg.pulse_shape_params);
  m_pulse = std::shared_ptr<SignalPulse>(std::move(uptr));

  // Enable caching with the timestep as the resolution only if supported (linear in charge)
  if (m_pulse->supportsCaching()) {
    m_pulse->enableCache(m_cfg.timestep);
  }

  m_min_sampling_time = std::max<double>(m_pulse->getMaximumTime(), m_cfg.min_sampling_time);
}

template <typename HitT>
void PulseGeneration<HitT>::process(
    const typename PulseGenerationAlgorithm<HitT>::Input& input,
    const typename PulseGenerationAlgorithm<HitT>::Output& output) const {
  const auto [simhits] = input;
  auto [rawPulses]     = output;

  for (const auto& hit : *simhits) {
    // Avoid repeated shared_ptr access overhead in hot path; keep read-only API here
    const SignalPulse* const pulsePtr = m_pulse.get();
    const auto [time, charge]         = HitAdapter<HitT>::getPulseSources(hit);
    // Calculate nearest timestep to the hit time rounded down (assume clocks aligned with time 0)
    double signal_time = m_cfg.timestep * std::floor(time / m_cfg.timestep);

    bool passed_threshold   = false;
    std::uint32_t skip_bins = 0;
    float integral          = 0;

    std::vector<float> pulse;
    pulse.reserve(m_cfg.max_time_bins);

    if (pulsePtr->cachingEnabled() && pulsePtr->supportsLinearInterpolation()) {
      // High-performance streaming with cached integer samples + linear interpolation
      auto state       = pulsePtr->prepareStreaming(signal_time, time, m_cfg.timestep);
      const double nt0 = (signal_time - time) / m_cfg.timestep;
      for (std::uint32_t i = 0; i < m_cfg.max_time_bins; i++) {
        double v_n    = state.v_floor_n + state.frac * (state.v_ceil_n - state.v_floor_n);
        double signal = charge * v_n;
        if (std::fabs(signal) < m_cfg.ignore_thres) {
          if (!passed_threshold) {
            skip_bins = i;
            // advance state and continue skipping
            state.base_index += 1;
            state.v_floor_n = state.v_ceil_n;
            state.v_ceil_n  = pulsePtr->sampleNormalizedAtIndex(state.base_index + 1);
            continue;
          }
          const double t = (static_cast<double>(i) - nt0) * m_cfg.timestep;
          if (t > m_min_sampling_time) {
            break;
          }
          // Below threshold after start: advance and continue without appending
          state.base_index += 1;
          state.v_floor_n = state.v_ceil_n;
          state.v_ceil_n  = pulsePtr->sampleNormalizedAtIndex(state.base_index + 1);
          continue;
        }
        passed_threshold = true;
        pulse.push_back(signal);
        integral += signal;

        // Advance streaming cache state for next iteration
        state.base_index += 1;
        state.v_floor_n = state.v_ceil_n;
        state.v_ceil_n  = pulsePtr->sampleNormalizedAtIndex(state.base_index + 1);
      }
    } else if (pulsePtr->cachingEnabled()) {
      // Caching is enabled, but interpolation is unsafe (e.g., discontinuities).
      // Use exact sampling at fractional offsets (cache used only for integer indices).
      auto state       = pulsePtr->prepareStreaming(signal_time, time, m_cfg.timestep);
      const double nt0 = (signal_time - time) / m_cfg.timestep;
      for (std::uint32_t i = 0; i < m_cfg.max_time_bins; i++) {
        const double v_n =
            pulsePtr->sampleNormalizedAt(static_cast<double>(state.base_index) + state.frac);
        double signal = charge * v_n;
        if (std::fabs(signal) < m_cfg.ignore_thres) {
          if (!passed_threshold) {
            skip_bins = i;
            state.base_index += 1;
            continue;
          }
          const double t = (static_cast<double>(i) - nt0) * m_cfg.timestep;
          if (t > m_min_sampling_time) {
            break;
          }
          state.base_index += 1;
          continue;
        }
        passed_threshold = true;
        pulse.push_back(signal);
        integral += signal;
        state.base_index += 1;
      }
    } else {
      // Fallback: exact evaluation per bin when caching/streaming is disabled
      const double nt0 = (signal_time - time) / m_cfg.timestep;
      for (std::uint32_t i = 0; i < m_cfg.max_time_bins; i++) {
        const double t_rel = (static_cast<double>(i) - nt0) * m_cfg.timestep;
        double signal      = (*m_pulse)(t_rel, charge);
        if (std::fabs(signal) < m_cfg.ignore_thres) {
          if (!passed_threshold) {
            skip_bins = i;
            continue;
          }
          if (t_rel > m_min_sampling_time) {
            break;
          }
          continue;
        }
        passed_threshold = true;
        pulse.push_back(signal);
        integral += signal;
      }
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
