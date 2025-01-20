// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#pragma once

#include "algorithms/onnx/DIRCParticleIDPreML.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class DIRCParticleIDPreML_factory : public JOmniFactory<DIRCParticleIDPreML_factory, NoConfig> {

public:
  using AlgoT = eicrecon::DIRCParticleIDPreML;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::RawTrackerHit> m_dirc_hit_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_track_input{this};
  PodioInput<edm4eic::MCRecoParticleAssociation> m_track_assoc_input{this};

  PodioOutput<edm4eic::Tensor> m_dirc_feature_tensor_output{this};
  PodioOutput<edm4eic::Tensor> m_track_feature_tensor_output{this};
  PodioOutput<edm4eic::Tensor> m_pid_target_tensor_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_dirc_hit_input(), m_track_input(), m_track_assoc_input()},
                    {m_dirc_feature_tensor_output().get(), m_track_feature_tensor_output().get(),
                     m_pid_target_tensor_output().get()});
  }
};

} // namespace eicrecon
