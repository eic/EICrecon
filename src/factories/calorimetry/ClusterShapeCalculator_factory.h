// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include "algorithms/calorimetry/ClusterShapeCalculator.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"



namespace eicrecon {

  class ClusterShapeCalculator_factory
    : public JOmniFactory<ClusterShapeCalculator_factory, ClusterShapeCalculatorConfig>
  {

    public:

      using AlgoT = eicrecon::ClusterShapeCalculator;

    private:

      // algorithm to run
      std::unique_ptr<AlgoT> m_algo;

      // input collections
      PodioInput<edm4eic::Cluster> m_clusters_input {this};
      PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_assocs_input {this};

      // output collections
      PodioOutput<edm4eic::Cluster> m_clusters_output {this};
      PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_assocs_output {this};

      // parameter bindings
      ParameterRef<bool> m_longitudinalShowerInfoAvailable {this, "longitudinalShowerInfoAvailable", config().longitudinalShowerInfoAvailable};

      // services
      Service<AlgorithmsInit_service> m_algoInitSvc {this};

    public:

      void Configure() {
        m_algo = std::make_unique<AlgoT>( GetPrefix() );
        m_algo->applyConfig( config() );
        m_algo->init();
      }

      void ChangeRun(int64_t run_number) {
        //... nothing to do ...//
      }

      void Process(int64_t run_number, int64_t event_number) {
        m_algo->process(
          {m_clusters_input(), m_assocs_input()},
          {m_clusters_output().get(), m_assocs_output().get()}
        );
      }

  };  // end ClusterShapeCalculator_factory

}  // end eicrecon namespace
