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
#include "algorithms/calorimetry/TrackClusterMergeSplitter.h"

namespace eicrecon {

class TrackClusterMergeSplitter_factory
    : public JOmniFactory<TrackClusterMergeSplitter_factory, TrackClusterMergeSplitterConfig> {

public:
  using AlgoT = eicrecon::TrackClusterMergeSplitter;

private:
  // algorithm to run
  std::unique_ptr<AlgoT> m_algo;

  // input collections
  PodioInput<edm4eic::ProtoCluster> m_protoclusters_input{this};
  PodioInput<edm4eic::TrackSegment> m_track_projections_input{this};

  // output collections
  PodioOutput<edm4eic::ProtoCluster> m_protoclusters_output{this};

  // parameter bindings
  ParameterRef<std::string> m_idCalo{this, "idCalo", config().idCalo};
  ParameterRef<double> m_minSigCut{this, "minSigCut", config().minSigCut};
  ParameterRef<double> m_avgEP{this, "avgEP", config().avgEP};
  ParameterRef<double> m_sigEP{this, "sigEP", config().sigEP};
  ParameterRef<double> m_drAdd{this, "drAdd", config().drAdd};
  ParameterRef<double> m_sampFrac{this, "sampFrac", config().sampFrac};
  ParameterRef<double> m_transverseEnergyProfileScale{this, "transverseEnergyProfileScale",
                                                      config().transverseEnergyProfileScale};

  // services
  Service<DD4hep_service> m_geoSvc{this};
  Service<AlgorithmsInit_service> m_algoInitSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_protoclusters_input(), m_track_projections_input()},
                    {m_protoclusters_output().get()});
  }

}; // end TrackClusterMergeSplitter_factory

} // namespace eicrecon
