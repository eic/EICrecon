// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#pragma once

// c++ utilities
#include <string>
// dd4hep utilities
#include <DD4hep/Detector.h>
// edm utilities
#include <edm4eic/EDM4eicVersion.h>
// eicrecon components
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
      PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_cluster_association_input {this};
#if EDM4EIC_VERSION_MAJOR >= 7
      PodioInput<edm4eic::MCRecoCalorimeterHitAssociation> m_hit_association_input {this};
#else
      PodioInput<edm4hep::SimCalorimeterHit> m_sim_hit_input {this};
#endif

      // output collections
      PodioOutput<edm4eic::Cluster> m_clusters_output {this};
#if EDM4EIC_VERSION_MAJOR >= 8
      PodioOutput<edm4eic::TrackClusterMatch> m_track_cluster_match_output {this};
#endif
      PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_cluster_association_output {this};

      // parameter bindings
      ParameterRef<std::string> m_idCalo {this, "idCalo", config().idCalo};
      ParameterRef<double> m_minSigCut {this, "minSigCut", config().minSigCut};
      ParameterRef<double> m_avgEP {this, "avgEP", config().avgEP};
      ParameterRef<double> m_sigEP {this, "sigEP", config().sigEP};
      ParameterRef<double> m_drAdd {this, "drAdd", config().drAdd};
      ParameterRef<double> m_sampFrac {this, "sampFrac", config().sampFrac};
      ParameterRef<double> m_transverseEnergyProfileScale {this, "transverseEnergyProfileScale", config().transverseEnergyProfileScale};

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
#if EDM4EIC_VERSION_MAJOR >= 7
          {m_clusters_input(), m_track_projections_input(), m_cluster_association_input(), m_hit_association_input()},
#else
          {m_clusters_input(), m_track_projections_input(), m_cluster_association_input(), m_sim_hit_input()},
#endif
#if EDM4EIC_VERSION_MAJOR >= 8
          {m_clusters_output().get(), m_track_cluster_match_output().get(), m_cluster_association_output().get()}
#else
          {m_clusters_output().get(), m_cluster_association_output().get()}
#endif
        );
      }

  };  // end TrackClusterMergeSplitter_factory

}  // end eicrecon namespace
