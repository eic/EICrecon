// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

#include "algorithms/fardetectors/FarDetectorTransportationPostML.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
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

  // Config parameter that also syncs from PODIO frame metadata at specified event level
  ParameterRef<float> m_beamE{this, "beamE", config().beamE,
                              "Beam energy in GeV",
                              PodioParameter<std::string>("electron_beam_energy", JEventLevel::Run)};
  ParameterRef<bool> m_requireBeamElectron{this, "requireBeamElectron",
                                           config().requireBeamElectron};
  ParameterRef<int> m_pdg_value{
      this, "pdgValue", config().pdg_value,
      "PDG value for the particle type to identify (default is electron)"};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  // BeginRun called after auto-sync of run parameters - just apply updated config
  void BeginRun(const std::shared_ptr<const JEvent>& /* event */) override {
    config().beamE_set_from_metadata = true;
    m_algo->applyConfig(config());
    logger()->info("beamE={} GeV (from run metadata)", config().beamE);
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) override {
    m_algo->process({m_prediction_tensor_input(), m_association_input(), m_beamelectrons_input()},
                    {m_particle_output().get(), m_association_output().get()});
  }
};

} // namespace eicrecon
