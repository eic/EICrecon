// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

#include "algorithms/fardetectors/FarDetectorTransportationPostML.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "services/io/podio/PodioRunFrame_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class FarDetectorTransportationPostML_factory
    : public JOmniFactory<FarDetectorTransportationPostML_factory,
                          FarDetectorTransportationPostMLConfig> {

public:
  using AlgoT = eicrecon::FarDetectorTransportationPostML;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Tensor> m_prediction_tensor_input{this};
  PodioInput<edm4eic::MCRecoTrackParticleAssociation> m_association_input{this};
  PodioInput<edm4hep::MCParticle> m_beamelectrons_input{this};

  PodioOutput<edm4eic::ReconstructedParticle> m_particle_output{this};
  PodioOutput<edm4eic::MCRecoParticleAssociation> m_association_output{this};

  ParameterRef<float> m_beamE{this, "beamE", config().beamE};
  ParameterRef<bool> m_requireBeamElectron{this, "requireBeamElectron",
                                           config().requireBeamElectron};
  ParameterRef<int> m_pdg_value{
      this, "pdgValue", config().pdg_value,
      "PDG value for the particle type to identify (default is electron)"};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};
  Service<PodioRunFrame_service> m_runFrameService{this};

public:
  void Configure() {

    // Try to get electron_beam_energy from run metadata
    if (auto beamEnergy = m_runFrameService().GetParameterAsDouble("electron_beam_energy")) {
      config().beamE = static_cast<float>(*beamEnergy);
      config().set_from_metadata = true;
      logger()->info("Using electron_beam_energy from run metadata: {} GeV", config().beamE);
    }

    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_prediction_tensor_input(), m_association_input(), m_beamelectrons_input()},
                    {m_particle_output().get(), m_association_output().get()});
  }
};

} // namespace eicrecon
