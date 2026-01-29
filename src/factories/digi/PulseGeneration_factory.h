// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy
//                         Minho Kim, Sylvester Joosten, Wouter Deconinck, Dmitry Kalinkin
//

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/digi/PulseGeneration.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <typename HitT>
class PulseGeneration_factory
    : public JOmniFactory<PulseGeneration_factory<HitT>, PulseGenerationConfig> {
public:
  using AlgoT    = eicrecon::PulseGeneration<HitT>;
  using FactoryT = JOmniFactory<PulseGeneration_factory<HitT>, PulseGenerationConfig>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template PodioInput<HitT> m_in_sim_hits{this};
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
  typename FactoryT::template PodioOutput<edm4eic::SimPulse> m_out_pulses{this};
#else
  typename FactoryT::template PodioOutput<edm4hep::TimeSeries> m_out_pulses{this};
#endif

  typename FactoryT::template ParameterRef<std::string> m_pulse_shape_function{
      this, "pulseShapeFunction", this->config().pulse_shape_function};
  typename FactoryT::template ParameterRef<std::vector<double>> m_pulse_shape_params{
      this, "pulseShapeParams", this->config().pulse_shape_params};
  typename FactoryT::template ParameterRef<double> m_timestep{this, "timestep",
                                                              this->config().timestep};
  typename FactoryT::template ParameterRef<double> m_ignore_thres{this, "ignoreThreshold",
                                                                  this->config().ignore_thres};
  typename FactoryT::template ParameterRef<double> m_min_sampling_time{
      this, "minSamplingTime", this->config().min_sampling_time};
  typename FactoryT::template ParameterRef<uint32_t> m_max_time_bins{this, "maxTimeBins",
                                                                     this->config().max_time_bins};

  typename FactoryT::template Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->applyConfig(this->config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_in_sim_hits()}, {m_out_pulses().get()});
  }
};

} // namespace eicrecon
