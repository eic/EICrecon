#pragma once
#include "src/algorithms/reco/ElectronReconstructionConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#ifndef EICRECON_FACTORY_PRECOMPILE

namespace eicrecon {
class ReconstructedElectrons_factory;
}

extern template class JOmniFactory<eicrecon::ReconstructedElectrons_factory, eicrecon::ElectronReconstructionConfig>;
extern template class JOmniFactoryGeneratorT<eicrecon::ReconstructedElectrons_factory>;

#else

#include "extensions/jana/JOmniFactory.h"
#include "algorithms/reco/ElectronReconstruction.h"

namespace eicrecon {

class ReconstructedElectrons_factory
    : public JOmniFactory<ReconstructedElectrons_factory, ElectronReconstructionConfig> {
public:
  using AlgoT = eicrecon::ElectronReconstruction;

private:
  // Underlying algorithm
  std::unique_ptr<AlgoT> m_algo;

  // Declare inputs
  PodioInput<edm4eic::ReconstructedParticle> m_in_rc_particles{this, "ReconstructedParticles"};

  // Declare outputs
  PodioOutput<edm4eic::ReconstructedParticle> m_out_reco_particles{this};

  // Declare parameters
  ParameterRef<double> m_min_energy_over_momentum{this, "minEnergyOverMomentum",
                                                  config().min_energy_over_momentum};
  ParameterRef<double> m_max_energy_over_momentum{this, "maxEnergyOverMomentum",
                                                  config().max_energy_over_momentum};

public:
  void Configure() {
    // This is called when the factory is instantiated.
    // Use this callback to make sure the algorithm is configured.
    // The logger, parameters, and services have all been fetched before this is called
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));

    // Pass config object to algorithm
    m_algo->applyConfig(config());

    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    // This is called on every event.
    // Use this callback to call your Algorithm using all inputs and outputs
    // The inputs will have already been fetched for you at this point.
    m_algo->process({m_in_rc_particles()}, {m_out_reco_particles().get()});

    logger()->debug("Found {} reconstructed electron candidates", m_out_reco_particles()->size());
  }
};

} // namespace eicrecon

#endif // EICRECON_FACTORY_PRECOMPILE
