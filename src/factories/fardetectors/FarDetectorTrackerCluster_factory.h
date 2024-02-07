// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Simon Gardner

#pragma once

#include "algorithms/fardetectors/FarDetectorTrackerCluster.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class FarDetectorTrackerCluster_factory :
public JOmniFactory<FarDetectorTrackerCluster_factory, FarDetectorTrackerClusterConfig> {

public:
  using AlgoT = eicrecon::FarDetectorTrackerCluster;
private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::RawTrackerHit> m_raw_hits_input {this};
  PodioOutput<edm4hep::TrackerHit>   m_clustered_hits_output {this};

  Service<DD4hep_service> m_geoSvc {this};

  ParameterRef<double> hit_time_limit {this, "time_limit", config().time_limit};
  
public:

  /** One time initialization **/
  void Configure() {

    m_algo = std::make_unique<AlgoT>(GetPrefix());
    // Setup algorithm
    m_algo->applyConfig(config());
    m_algo->init(m_geoSvc().converter(),m_geoSvc().detector(),logger());

  }


  void ChangeRun(int64_t run_number) {
  }

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_raw_hits_input()}, {m_clustered_hits_output().get()});
  }

};

} // eicrecon
