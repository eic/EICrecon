// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Minho Kim

#pragma once

#include "algorithms/calorimetry/EdepToNpeConversion.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class EdepToNpeConversion_factory
    : public JOmniFactory<EdepToNpeConversion_factory, EdepToNpeConversionConfig> {

public:
  using AlgoT = eicrecon::EdepToNpeConversion;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::EventHeader> m_in_headers{this};
  PodioInput<edm4hep::SimCalorimeterHit> m_in_hits{this};
  PodioOutput<edm4hep::SimCalorimeterHit> m_out_hits{this};

  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::vector<std::string>> m_edep_to_npe_fields{this, "edepToNpeFields",
                                                              config().edep_to_npe_fields};
  ParameterRef<std::string> m_edep_to_npe_filename{this, "edepToNpeFilename",
                                                   config().edep_to_npe_filename};
  ParameterRef<double> m_edep_to_npe{this, "edepToNpe", config().edep_to_npe};

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
