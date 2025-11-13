// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include <string>

#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "algorithms/calorimetry/TrackClusterMergeSplitter.h"

namespace eicrecon {

class TrackClusterMergeSplitter_factory
    : public JOmniFactory<TrackClusterMergeSplitter_factory, TrackClusterMergeSplitterConfig> {

public:
  ///! alias for algorithm name
  using AlgoT = eicrecon::TrackClusterMergeSplitter;

private:
  // pointer to algorithm
  std::unique_ptr<AlgoT> m_algo;

  // input collections
  PodioInput<edm4eic::TrackClusterMatch> m_track_cluster_matches_input{this};
  PodioInput<edm4eic::Cluster> m_clusters_input{this};
  PodioInput<edm4eic::TrackSegment> m_track_projections_input{this};

  // output collections
  PodioOutput<edm4eic::ProtoCluster> m_protoclusters_output{this};
#if EDM4EIC_VERSION_MAJOR >= 8 && EDM4EIC_VERSION_MINOR >= 4
  PodioOutput<edm4eic::TrackProtoClusterMatch> m_track_protocluster_match_output{this};
#endif

  // parameter bindings
  ParameterRef<double> m_minSigCut{this, "minSigCut", config().minSigCut};
  ParameterRef<double> m_avgEP{this, "avgEP", config().avgEP};
  ParameterRef<double> m_sigEP{this, "sigEP", config().sigEP};
  ParameterRef<double> m_drAdd{this, "drAdd", config().drAdd};
  ParameterRef<uint64_t> m_surfaceToUse{this, "surfaceToUse", config().surfaceToUse};
  ParameterRef<double> m_transverseEnergyProfileScale{this, "transverseEnergyProfileScale",
                                                      config().transverseEnergyProfileScale};

  // services
  Service<AlgorithmsInit_service> m_algoInitSvc{this};

public:
  ///! Configures algorithm
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  ///! Primary algorithm call
  void Process(int64_t /*run_number*/, uint64_t /*event_number*/) {
    m_algo->process(
        {m_track_cluster_matches_input(), m_clusters_input(), m_track_projections_input()},
#if EDM4EIC_VERSION_MAJOR >= 8 && EDM4EIC_VERSION_MINOR >= 4
        { m_protoclusters_output().get(), m_track_protocluster_match_output().get() }
#else
        {m_protoclusters_output().get()}
#endif
    );
  }

}; // end TrackClusterMergeSplitter_factory

} // namespace eicrecon
