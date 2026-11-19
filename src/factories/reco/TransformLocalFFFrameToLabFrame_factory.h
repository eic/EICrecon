// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Alex Jentsch

#pragma once

#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "algorithms/reco/TransformLocalFFFrameToLabFrame.h"

namespace eicrecon {

class TransformLocalFFFrameToLabFrame_factory
    : public JOmniFactory<TransformLocalFFFrameToLabFrame_factory,
                          TransformLocalFFFrameToLabFrameConfig> {

public:
  using AlgoT = eicrecon::TransformLocalFFFrameToLabFrame;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::ReconstructedParticle> m_FF_local_input{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_FF_lab_output{this};

  ParameterRef<double> m_crossing_angle{this, "crossing_angle", config().m_crossing_angle};

  // services
  Service<AlgorithmsInit_service> m_algoInitSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_FF_local_input()}, {m_FF_lab_output().get()});
  }
};

} // namespace eicrecon
