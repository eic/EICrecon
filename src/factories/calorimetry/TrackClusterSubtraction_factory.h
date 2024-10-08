// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#pragma once

// c++ utilities
#include <string>
// dd4hep utilities
#include <DD4hep/Detector.h>
// eicrecon components
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "algorithms/calorimetry/TrackClusterSubtraction.h"

namespace eicrecon {

  class TrackClusterSubtraction_factory : public JOmniFactory<TrackClusterSubtraction_factory, TrackClusterSubtractionConfig> {

    public:

      using AlgoT = eicrecon::TrackClusterSubtraction;

    private:

      // algorithm to run
      std::unique_ptr<AlgoT> m_algo;

      // input collections
      PodioInput<edm4eic::Cluster> m_emclusters_input {this};
      PodioInput<edm4eic::Cluster> m_hclusters_input {this};
      PodioInput<edm4eic::TrackSegment> m_track_projections_input {this};

      // output collections
      PodioOutput<edm4eic::Cluster> m_emclusters_output {this};
      PodioOutput<edm4eic::Cluster> m_hclusters_output {this};

      // parameter bindings
      /* TODO fill in */

      // services
      Service<DD4hep_service> m_geoSvc {this};
      Service<AlgorithmsInit_service> m_algoInitSvc {this};

    public:

      void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->applyConfig( config() );
        m_algo->init(m_geoSvc().detector());
      }

      void ChangeRun(int64_t run_number) {
        /* nothing to do here */
      }

      void Process(int64_t run_number, uint64_t event_number) {
        m_algo->process(
          {m_emclusters_input(), m_hclusters_input(), m_track_projections_input()},
          {m_emclusters_output().get(), m_hclusters_output().get()}
        );
      }

  };  // end TrackClusterSubtraction_factory

}  // end eicrecon namespace
