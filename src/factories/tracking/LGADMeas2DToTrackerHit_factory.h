// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include "algorithms/tracking/LGADMeas2DToTrackerHit.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class LGADMeas2DToTrackerHit_factory
    : public JOmniFactory<LGADMeas2DToTrackerHit_factory> {
private:
  std::unique_ptr<eicrecon::LGADMeas2DToTrackerHit> m_algo;

  PodioInput<edm4eic::Measurement2D> m_meas2D_input{this};
  PodioOutput<edm4eic::TrackerHit> m_hits_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::LGADMeas2DToTrackerHit>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_meas2D_input()}, {m_hits_output().get()});
  }
};

} // namespace eicrecon
