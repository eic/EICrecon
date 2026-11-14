// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <ActsPodioEdm/TrackerHitLocalCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <memory>

#include "algorithms/tracking/TrackerHitLocalFromMeasurements.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class TrackerHitLocalFromMeasurements_factory
    : public JOmniFactory<TrackerHitLocalFromMeasurements_factory, NoConfig> {
public:
  using AlgoT = eicrecon::TrackerHitLocalFromMeasurements;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Measurement2D> m_measurements_input{this};
  PodioOutput<ActsPodioEdm::TrackerHitLocal> m_hits_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_measurements_input()}, {m_hits_output().get()});
  }
};

} // namespace eicrecon
