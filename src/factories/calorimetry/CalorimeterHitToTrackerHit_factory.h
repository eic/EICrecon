// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterHitToTrackerHit.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterHitToTrackerHit_factory : public JOmniFactory<CalorimeterHitToTrackerHit_factory> {

private:
public:
  using AlgoT = eicrecon::CalorimeterHitToTrackerHit;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::CalorimeterHit> m_calorimeter_hits_input{this};
  PodioOutput<edm4eic::TrackerHit> m_tracker_hits_output{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_calorimeter_hits_input()}, {m_tracker_hits_output().get()});
  }
};

} // namespace eicrecon
