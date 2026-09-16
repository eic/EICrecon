// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Simon Gardner

#pragma once

#include "algorithms/fardetectors/FarDetectorTrackerCluster.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class FarDetectorTrackerCluster_factory
    : public JOmniFactory<FarDetectorTrackerCluster_factory, FarDetectorTrackerClusterConfig> {

public:
  using AlgoT = eicrecon::FarDetectorTrackerCluster;

private:
  std::unique_ptr<AlgoT> m_algo;

  VariadicPodioInput<edm4eic::TrackerHit> m_raw_hits_input{this};
  VariadicPodioOutput<edm4eic::Measurement2D> m_clustered_hits_output{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

  ParameterRef<std::string> m_readout{this, "readoutClass", config().readout};
  ParameterRef<std::string> m_x_field{this, "xField", config().x_field};
  ParameterRef<std::string> m_y_field{this, "yField", config().y_field};
  ParameterRef<double> m_hit_time_limit{this, "hitTimeLimit", config().hit_time_limit};

public:
  /** One time initialization **/
  void Configure() {

    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    // Setup algorithm
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    std::vector<gsl::not_null<edm4eic::Measurement2DCollection*>> clustered_collections;
    for (const auto& clustered : m_clustered_hits_output()) {
      clustered_collections.push_back(
          gsl::not_null<edm4eic::Measurement2DCollection*>(clustered.get()));
    }

    auto in1 = m_raw_hits_input();
    std::vector<gsl::not_null<const edm4eic::TrackerHitCollection*>> in2;
    std::copy(in1.cbegin(), in1.cend(), std::back_inserter(in2));

    m_algo->process(in2, clustered_collections);
  }
};

} // namespace eicrecon
