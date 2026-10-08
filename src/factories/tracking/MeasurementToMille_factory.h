// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 ePIC Collaboration

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/AlignmentDerivativeSetCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/MeasurementToMille.h"
#include "algorithms/tracking/MeasurementToMilleConfig.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class MeasurementToMille_factory
    : public JOmniFactory<MeasurementToMille_factory, MeasurementToMilleConfig> {

private:
  using AlgoT = eicrecon::MeasurementToMille;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Track> m_tracks_input{this};
  PodioInput<edm4eic::Measurement2D> m_measurements_input{this};
  PodioOutput<edm4eic::AlignmentDerivativeSet> m_derivatives_output{this};

  ParameterRef<float> m_maxChi2PerNDF{this, "maxChi2PerNDF", config().maxChi2PerNDF,
                                      "Maximum chi2/NDF for tracks used in alignment"};
  ParameterRef<float> m_minMomentum{this, "minMomentum", config().minMomentum,
                                    "Minimum track momentum [GeV/c]"};
  ParameterRef<std::vector<int>> m_fixedLayers{this, "fixedLayers", config().fixedLayers,
                                               "0-based layer indices to fix in Millepede"};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_tracks_input(), m_measurements_input()}, {m_derivatives_output().get()});
  }
};

} // namespace eicrecon
