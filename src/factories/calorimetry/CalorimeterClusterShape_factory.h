// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include "algorithms/calorimetry/CalorimeterClusterShape.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CalorimeterClusterShape_factory
    : public JOmniFactory<CalorimeterClusterShape_factory, CalorimeterClusterShapeConfig> {

public:
  using AlgoT = eicrecon::CalorimeterClusterShape;

private:
  // algorithm to run
  std::unique_ptr<AlgoT> m_algo;

  // input collections
  PodioInput<edm4eic::Cluster> m_clusters_input{this};
  PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_assocs_input{this};

  // output collections
  PodioOutput<edm4eic::Cluster> m_clusters_output{this};
  PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_assocs_output{this};

  // parameter bindings
  ParameterRef<bool> m_longitudinalShowerInfoAvailable{this, "longitudinalShowerInfoAvailable",
                                                       config().longitudinalShowerInfoAvailable};
  ParameterRef<std::string> m_energyWeight{this, "energyWeight", config().energyWeight};
  ParameterRef<double> m_sampFrac{this, "sampFrac", config().sampFrac};
  ParameterRef<std::vector<double>> m_logWeightBaseCoeffs{this, "logWeightBaseCoeffs",
                                                          config().logWeightBaseCoeffs};
  ParameterRef<double> m_logWeightBase_Eref{this, "logWeightBase_Eref",
                                            config().logWeightBase_Eref};
  ParameterRef<double> m_logWeightBase{this, "logWeightBase", config().logWeightBase};

  // services
  Service<AlgorithmsInit_service> m_algoInitSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_clusters_input(), m_assocs_input()},
                    {m_clusters_output().get(), m_assocs_output().get()});
  }

}; // end CalorimeterClusterShape_factory

} // namespace eicrecon
