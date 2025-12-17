// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

#include "algorithms/fardetectors/FarDetectorTransportationPreML.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class FarDetectorTransportationPreML_factory
    : public JOmniFactory<FarDetectorTransportationPreML_factory,
                          FarDetectorTransportationPreMLConfig> {

public:
  using AlgoT = eicrecon::FarDetectorTransportationPreML;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Track> m_track_input{this};
  PodioInput<edm4eic::MCRecoTrackParticleAssociation> m_association_input{this};
  PodioInput<edm4hep::MCParticle> m_beamelectrons_input{this};

  PodioOutput<edm4eic::Tensor> m_feature_tensor_output{this};
  PodioOutput<edm4eic::Tensor> m_target_tensor_output{this};

  ParameterRef<float> m_beamE{this, "beamE", config().beamE};
  ParameterRef<bool> m_requireBeamElectron{this, "requireBeamElectron",
                                           config().requireBeamElectron};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }
  
  // Initialize beam energy from parent run frame if available
  void BeginRun(const std::shared_ptr<const JEvent>& event) override {

    // std::cout << this->GetPreviousRunNumber() << " " << this->GetPreviousEventNumber() << std::endl;
    // Try to access parent run event and extract electron_beam_energy
    float beamE = config().beamE;
    try {
      const JEvent& parent = event->GetParent(JEventLevel::Run);
      const auto* run_frame = parent.GetSingle<podio::Frame>();
      if (run_frame != nullptr) {
        // Parameter currently stored as string
        std::optional<std::string> s_opt = run_frame->getParameter<std::string>("electron_beam_energy");
        if (s_opt.has_value()) {
          try {
            beamE = static_cast<float>(std::stod(s_opt.value()));
          } catch (...) {
            // Keep default on parse failure
          }
        }
      }
    } catch (std::exception& e) {
      // Keep default beamE if any exception occurs
      logger()->debug("BeginRun: unable to read electron_beam_energy from run frame: {}", e.what());
    }
    config().beamE = beamE;
    config().beamE_set_from_metadata = true;
    m_algo->applyConfig(config());
    logger()->info("beamE={} GeV (from run metadata)", beamE);
    // Call base to keep resources and algorithm ChangeRun() in sync
    // JOmniFactory<FarDetectorTransportationPreML_factory, FarDetectorTransportationPreMLConfig>::BeginRun(event);
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) override{
    m_algo->process({m_track_input(), m_association_input(), m_beamelectrons_input()},
                    {m_feature_tensor_output().get(), m_target_tensor_output().get()});
  }
};

} // namespace eicrecon
