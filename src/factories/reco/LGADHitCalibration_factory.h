// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include "algorithms/reco/LGADHitCalibration.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class LGADHitCalibration_factory
    : public JOmniFactory<LGADHitCalibration_factory, LGADHitCalibrationConfig> {
private:
  std::unique_ptr<eicrecon::LGADHitCalibration> m_algo;

  PodioInput<edm4eic::RawTrackerHit> m_raw_hits_input{this};
  PodioOutput<edm4eic::TrackerHit> m_rec_hits_output{this};

  ParameterRef<double> m_c_slope{this, "cSlope", config().c_slope};
  ParameterRef<double> m_c_intercept{this, "cIntercept", config().c_intercept};
  ParameterRef<double> m_t_slope{this, "tSlope", config().t_slope};
  ParameterRef<double> m_t_intercept{this, "tIntercept", config().t_intercept};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::LGADHitCalibration>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_raw_hits_input()}, {m_rec_hits_output().get()});
  }
};

} // namespace eicrecon
