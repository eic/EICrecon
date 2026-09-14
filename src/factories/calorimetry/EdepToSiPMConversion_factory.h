// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once
#include "algorithms/calorimetry/EdepToSiPMConversion.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class EdepToSiPMConversion_factory
    : public JOmniFactory<EdepToSiPMConversion_factory, EdepToSiPMConversionConfig> {

public:
  using AlgoT = eicrecon::EdepToSiPMConversion;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::EventHeader> m_in_headers{this};
  PodioInput<edm4hep::SimCalorimeterHit> m_in_hits{this};
  PodioOutput<edm4hep::SimCalorimeterHit> m_out_hits{this};

  ParameterRef<double> m_edep_to_npe{this, "edepToNpe", config().edep_to_npe};
  ParameterRef<unsigned long long> m_num_effective_sipm_pixels{
      this, "numEffectiveSipmPixels", config().num_effective_sipm_pixels};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_in_headers(), m_in_hits()}, {m_out_hits().get()});
  }
};

} // namespace eicrecon
