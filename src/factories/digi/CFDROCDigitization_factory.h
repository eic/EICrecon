#pragma once
#include "algorithms/digi/CFDROCDigitizationConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#ifndef EICRECON_FACTORY_PRECOMPILE

namespace eicrecon {
class CFDROCDigitization_factory;
}

extern template class JOmniFactory<eicrecon::CFDROCDigitization_factory, eicrecon::CFDROCDigitizationConfig>;
extern template class JOmniFactoryGeneratorT<eicrecon::CFDROCDigitization_factory>;

#else

#include <iostream>
#include "algorithms/digi/CFDROCDigitization.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CFDROCDigitization_factory
    : public JOmniFactory<CFDROCDigitization_factory, CFDROCDigitizationConfig> {
public:
  using AlgoT = eicrecon::CFDROCDigitization;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::RawTimeSeries> m_in_sim_track{this};

  PodioOutput<edm4eic::RawTrackerHit> m_out_reco_particles{this};

  ParameterRef<double> m_fraction{this, "fraction", config().fraction};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::CFDROCDigitization>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_in_sim_track()}, {m_out_reco_particles().get()});
  }
};

} // namespace eicrecon

#endif // EICRECON_FACTORY_PRECOMPILE
