// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025 Wouter Deconinck

#pragma once

#include "algorithms/tracking/MPGDHitReconstruction.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class MPGDHitReconstruction_factory
    : public JOmniFactory<MPGDHitReconstruction_factory, MPGDHitReconstructionConfig> {

public:
  using AlgoT = eicrecon::MPGDHitReconstruction;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::RawTrackerHit> m_raw_hits_input{this};
  PodioOutput<edm4eic::TrackerHit> m_rec_hits_output{this};

  ParameterRef<float> m_timeResolution{this, "timeResolution", config().timeResolution};

  Service<DD4hep_service> m_geoSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_raw_hits_input()}, {m_rec_hits_output().get()});
  }
};

} // namespace eicrecon
