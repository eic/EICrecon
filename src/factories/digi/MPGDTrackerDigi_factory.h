// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2024 Wouter Deconinck, Yann Bedfer

#pragma once

#include "algorithms/digi/MPGDTrackerDigi.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"

namespace eicrecon {

class MPGDTrackerDigi_factory
    : public JOmniFactory<MPGDTrackerDigi_factory, MPGDTrackerDigiConfig> {

public:
  using AlgoT = eicrecon::MPGDTrackerDigi;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::EventHeader> m_event_headers_input{this};
  PodioInput<edm4hep::SimTrackerHit> m_sim_hits_input{this};

  PodioOutput<edm4eic::RawTrackerHit> m_raw_hits_output{this};
  PodioOutput<edm4eic::MCRecoTrackerHitAssociation> m_assoc_output{this};

  Service<ACTSGeo_service> m_ACTSGeoSvc {this};

  ParameterRef<double> m_threshold{this, "threshold", config().threshold};
  ParameterRef<double> m_timeResolution{this, "timeResolution", config().timeResolution};
  ParameterRef<std::array<double,2>> m_stripResolutions {this, "stripResolutions", config().stripResolutions, "Space resolutions for p/n strips"};
  ParameterRef<std::array<int,2>>    m_stripNumbers {this, "stripNumbers", config().stripNumbers, "Number of p/n strips per module"};
  ParameterRef<std::string> m_readout{this, "readoutClass", config().readout};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_event_headers_input(), m_sim_hits_input()},
                    {m_raw_hits_output().get(), m_assoc_output().get()});
  }
};

} // namespace eicrecon
