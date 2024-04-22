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

  VariadicPodioInput<edm4eic::RawTrackerHit> m_raw_hits_input {this};
  VariadicPodioOutput<edm4hep::TrackerHit>   m_clustered_hits_output {this};

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
    std::vector<gsl::not_null<edm4hep::TrackerHitCollection*>> clustered_collections;
    for (const auto& clustered : m_clustered_hits_output()) {
      clustered_collections.push_back(gsl::not_null<edm4hep::TrackerHitCollection*>(clustered.get()));
    }
    
    auto in1 = m_raw_hits_input();
    std::vector<gsl::not_null<const edm4eic::RawTrackerHitCollection*>> in2;
    std::copy(in1.cbegin(), in1.cend(), std::back_inserter(in2));


    m_algo->process(in2, clustered_collections);
  }

};

} // eicrecon
