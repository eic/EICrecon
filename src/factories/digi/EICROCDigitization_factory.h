#pragma once
#include "src/algorithms/digi/EICROCDigitizationConfig.h"
#include "extensions/jana/JOmniFactory.h"
#ifndef EICRECON_FACTORY_PRECOMPILE

namespace eicrecon {
class EICROCDigitization_factory;
}

extern template class JOmniFactory<eicrecon::EICROCDigitization_factory, eicrecon::EICROCDigitizationConfig>;

#else

#include <iostream>
#include "algorithms/digi/EICROCDigitization.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class EICROCDigitization_factory
    : public JOmniFactory<EICROCDigitization_factory, EICROCDigitizationConfig> {
public:
  using AlgoT = eicrecon::EICROCDigitization;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::RawTimeSeries> m_in_sim_track{this};

  PodioOutput<edm4eic::RawTrackerHit> m_out_reco_particles{this};

  ParameterRef<double> m_t_thres{this, "tThreshold", config().t_thres};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::EICROCDigitization>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_in_sim_track()}, {m_out_reco_particles().get()});
  }
};

} // namespace eicrecon

#endif // EICRECON_FACTORY_PRECOMPILE
