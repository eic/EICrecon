// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <DD4hep/Detector.h>
#include <string>

#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "algorithms/particle/TrackClusterSubtractor.h"

namespace eicrecon {

class TrackClusterSubtractor_factory
    : public JOmniFactory<TrackClusterSubtractor_factory, TrackClusterSubtractorConfig> {

public:
  ///! alias for algorithm name
  using AlgoT = eicrecon::TrackClusterSubtractor;

private:
  // pointer to algorithm
  std::unique_ptr<AlgoT> m_algo;

  // input collections
  PodioInput<edm4eic::TrackClusterMatch> m_track_cluster_match_input{this};
  PodioInput<edm4eic::Cluster> m_clusters_input{this};
  PodioInput<edm4eic::TrackSegment> m_track_projections_input{this};

  // output collections
  PodioOutput<edm4eic::Cluster> m_remnant_clusters_output{this};
  PodioOutput<edm4eic::Cluster> m_expected_clusters_output{this};
  PodioOutput<edm4eic::TrackClusterMatch> m_track_expected_match_output{this};

  // parameter bindings
  ParameterRef<double> m_fracEnergyToSub{this, "fracEnergyToSub", config().fracEnergyToSub};
  ParameterRef<int32_t> m_defaultMassPdg{this, "defaultMassPdg", config().defaultMassPdg};
  ParameterRef<uint64_t> m_surfaceToUse{this, "surfaceToUse", config().surfaceToUse};
  ParameterRef<bool> m_doNSigmaCut{this, "doNSigmaCut", config().doNSigmaCut};
  ParameterRef<uint32_t> m_nSigmaMax{this, "nSigmaMax", config().nSigmaMax};
  ParameterRef<double> m_trkReso{this, "trkReso", config().trkReso};
  ParameterRef<double> m_calReso{this, "calReso", config().calReso};

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
  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    m_algo->process(
        {m_track_cluster_match_input(), m_clusters_input(), m_track_projections_input()},
        {m_remnant_clusters_output().get(), m_expected_clusters_output().get(),
         m_track_expected_match_output().get()});
  }
}; // end TrackClusterSubtractor_factory

} // namespace eicrecon
