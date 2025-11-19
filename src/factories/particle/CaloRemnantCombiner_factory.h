// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Subhadip Pal

#pragma once

#include <algorithms/logger.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>

#include <spdlog/logger.h>
#include <stdint.h>
#include <memory>

#include "algorithms/particle/CaloRemnantCombiner.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CaloRemnantCombiner_factory
    : public JOmniFactory<CaloRemnantCombiner_factory, CaloRemnantCombinerConfig> {
private:
  // Underlying algorithm
  std::unique_ptr<eicrecon::CaloRemnantCombiner> m_algo;

  // Declare inputs
  VariadicPodioInput<edm4eic::Cluster> m_in_calo_clusters{this};

  // Declare outputs
  PodioOutput<edm4eic::ReconstructedParticle> m_out_neutral_candidates{this};

  // Declare parameters
  /*ParameterRef<double> m_min_energy_over_momentum{this, "minEnergyOverMomentum",
                                                  config().min_energy_over_momentum};
  ParameterRef<double> m_max_energy_over_momentum{this, "maxEnergyOverMomentum",
                                                  config().max_energy_over_momentum};*/

public:
  void Configure() {
    // This is called when the factory is instantiated.
    // Use this callback to make sure the algorithm is configured.
    // The logger, parameters, and services have all been fetched before this is called
    m_algo = std::make_unique<CaloRemnantCombiner>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    // Pass config object to algorithm
    m_algo->applyConfig(config());

    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    // This is called on every event.
    // Use this callback to call your Algorithm using all inputs and outputs
    // The inputs will have already been fetched for you at this point.
    auto in1 = m_in_calo_clusters();
    std::vector<gsl::not_null<const edm4eic::ClusterCollection*>> in2;

    std::copy(in1.cbegin(), in1.cend(), std::back_inserter(in2));
    m_algo->process({in2}, {m_out_neutral_candidates().get()});

    logger()->debug("Found {} reconstructed neutral candidates",
                    m_out_neutral_candidates()->size());
  }
};
} // namespace eicrecon
