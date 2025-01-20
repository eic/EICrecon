// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

#include "algorithms/fardetectors/FarDetectorTransportationPreML.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

class FarDetectorTransportationPreML_factory : public JOmniFactory<FarDetectorTransportationPreML_factory, FarDetectorTransportationPreMLConfig> {

public:
  using AlgoT = eicrecon::FarDetectorTransportationPreML;
private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::TrackParameters>  m_trackparam_input      {this};
  PodioInput<edm4hep::MCParticle>       m_scatteredelectrons_input {this};
  PodioInput<edm4hep::MCParticle>       m_beamelectrons_input   {this};

  PodioOutput<edm4eic::Tensor>          m_feature_tensor_output {this};
  PodioOutput<edm4eic::Tensor>          m_target_tensor_output  {this};

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
        m_algo->process({m_trackparam_input(),m_scatteredelectrons_input(), m_beamelectrons_input()},
                                        {m_feature_tensor_output().get(), m_target_tensor_output().get()});
  }
};

} // eicrecon
