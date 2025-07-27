// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023-2025, Simon Gardner

#pragma once

// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackCollection.h>
#include <algorithms/fardetectors/FarDetectorLinearProjection.h>
#include <extensions/jana/JOmniFactory.h>
#include <spdlog/logger.h>

namespace eicrecon {

class FarDetectorLinearProjection_factory
    : public JOmniFactory<FarDetectorLinearProjection_factory, FarDetectorLinearProjectionConfig> {

public:
  using AlgoT = eicrecon::FarDetectorLinearProjection;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Track> m_tracks_input{this};
  PodioOutput<edm4eic::TrackParameters> m_tracks_output{this};

  ParameterRef<std::vector<float>> plane_position{this, "planePosition", config().plane_position};
  ParameterRef<std::vector<float>> plane_a{this, "planeA", config().plane_a};
  ParameterRef<std::vector<float>> plane_b{this, "planeB", config().plane_b};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_tracks_input()}, {m_tracks_output().get()});
  }
};

} // namespace eicrecon
