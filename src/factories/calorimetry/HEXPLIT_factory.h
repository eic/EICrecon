// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

#pragma once

#include "algorithms/calorimetry/HEXPLIT.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class HEXPLIT_factory : public JOmniFactory<HEXPLIT_factory, HEXPLITConfig> {

  using AlgoT = eicrecon::HEXPLIT;

private:
  std::unique_ptr<AlgoT> m_algo;
  PodioInput<edm4eic::CalorimeterHit> m_rec_hits_input{this};
  PodioOutput<edm4eic::CalorimeterHit> m_subcell_hits_output{this};

  ParameterRef<double> m_MIP{this, "MIP", config().MIP};
  ParameterRef<double> m_Emin_in_MIPs{this, "Emin_in_MIPs", config().Emin_in_MIPs};
  ParameterRef<double> m_delta_in_MIPs{this, "delta_in_MIPs", config().delta_in_MIPs};
  ParameterRef<double> m_tmax{this, "tmax", config().tmax};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_rec_hits_input()}, {m_subcell_hits_output().get()});
  }
};

} // namespace eicrecon
