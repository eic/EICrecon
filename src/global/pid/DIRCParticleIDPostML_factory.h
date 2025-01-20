// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, Dmitry Kalinkin

#pragma once

#include "algorithms/onnx/DIRCParticleIDPostML.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

class DIRCParticleIDPostML_factory : public JOmniFactory<DIRCParticleIDPostML_factory, NoConfig> {

public:
  using AlgoT = eicrecon::DIRCParticleIDPostML;
private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::ReconstructedParticle> m_particle_input {this};
  PodioInput<edm4eic::MCRecoParticleAssociation> m_particle_assoc_input {this};
  PodioInput<edm4eic::Tensor> m_prediction_tensor_input {this};

  PodioOutput<edm4eic::ReconstructedParticle> m_particle_output {this};
  PodioOutput<edm4eic::MCRecoParticleAssociation> m_particle_assoc_output {this};
  PodioOutput<edm4hep::ParticleID> m_particle_id_output {this};

public:
  void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
        m_algo->applyConfig(config());
        m_algo->init();
  }

  void ChangeRun(int64_t run_number) {
  }

  void Process(int64_t run_number, uint64_t event_number) {
        m_algo->process({m_particle_input(), m_particle_assoc_input(), m_prediction_tensor_input()},
                        {m_particle_output().get(), m_particle_assoc_output().get(), m_particle_id_output().get()});
  }
};

} // eicrecon
