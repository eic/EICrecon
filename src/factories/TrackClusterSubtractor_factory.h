// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <DD4hep/Detector.h>
#include <edm4eic/EDM4eicVersion.h>
#include <string>

#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "algorithms/particle_flow/TrackClusterSubtractor.h"

namespace eicrecon {

class TrackClusterSubtractor_factory
    : public JOmniFactory<TrackClusterSubtractor_factory, TrackClusterSubtractorConfig> {

public:
  using AlgoT = eicrecon::TrackClusterSubtractor;

private:
  std::unique_ptr<AlgoT> m_algo;

  // input collections
  PodioInput<edm4eic::TrackClusterMatch> m_track_cluster_matches_input{this};
  PodioInput<edm4eic::Cluster> m_clusters_input{this};
  PodioInput<edm4eic::TrackSegment> m_track_projections_input{this};

  // output collections
  PodioOutput<edm4eic::Cluster> m_remnant_clusters_output{this};
  PodioOutput<edm4eic::Cluster> m_expected_clusters_output{this};
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  PodioOutput<edm4eic::TrackClusterLink> m_track_expected_links_output{this};
#else
  PodioOutput<edm4eic::TrackClusterMatch> m_track_expected_matches_output{this};
#endif

  // parameter bindings
  ParameterRef<double> m_energyFractionToSubtract{this, "energyFractionToSubtract",
                                                  config().energyFractionToSubtract};
  ParameterRef<int32_t> m_defaultPDG{this, "defaultPDG", config().defaultPDG};
  ParameterRef<uint8_t> m_surfaceToUse{this, "surfaceToUse", config().surfaceToUse};
  ParameterRef<bool> m_doNSigmaCut{this, "doNSigmaCut", config().doNSigmaCut};
  ParameterRef<uint32_t> m_nSigmaMax{this, "nSigmaMax", config().nSigmaMax};
  ParameterRef<double> m_calorimeterResolution{this, "calorimeterResolution",
                                               config().calorimeterResolution};

  // services
  Service<AlgorithmsInit_service> m_algoInitSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
    m_algo->process(
        {m_track_cluster_matches_input(), m_clusters_input(), m_track_projections_input()},
        {m_remnant_clusters_output().get(), m_expected_clusters_output().get(),
         m_track_expected_links_output().get()});
#else
    m_algo->process(
        {m_track_cluster_matches_input(), m_clusters_input(), m_track_projections_input()},
        {m_remnant_clusters_output().get(), m_expected_clusters_output().get(),
         m_track_expected_matches_output().get()});
#endif
  }
}; // end TrackClusterSubtractor_factory

} // namespace eicrecon
