// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#pragma once

#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "algorithms/calorimetry/TrackClusterMergeSplitter.h"

namespace eicrecon {

  class TrackClusterMergeSplitter_factory : public JOmniFactory<TrackClusterMergeSplitter_factory, TrackClusterMergeSplitterConfig> {

    public:

      using AlgoT = eicrecon::TrackClusterMergeSplitter;

    private:

      // algorithm to run
      std::unique_ptr<AlgoT> m_algo;

      // input collections
      PodioInput<edm4eic::Cluster> m_clusters_input {this};
      PodioInput<edm4eic::TrackSegment> m_track_projections_input {this};

      // output collections
      PodioOutput<edm4eic::Cluster> m_clusters_output {this};

      // parameter bindings
      ParameterRef<double> m_minSigCut {this, "", config().minSigCut};
      ParameterRef<double> m_avgEP {this, "", config().avgEP};
      ParameterRef<double> m_sigEP {this, "", config().sigEP};

      // services
      Service<DD4hep_service> m_geoSvc {this};
      Service<AlgorithmsInit_service> m_algoInitSvc {this};

    public:

      void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo -> applyConfig( config() );
        m_algo -> init();
      }

      void ChangeRun(int64_t run_number) {
        /* nothing to do here */
      }

      void Process(int64_t run_number, uint64_t event_number) {
        m_algo -> process(
          {m_clusters_input(), m_track_projections_input()},
          {m_clusters_output().get()}
        );
      }

  };  // end TrackClusterMergeSplitter_factory

}  // end eicrecon namespace
