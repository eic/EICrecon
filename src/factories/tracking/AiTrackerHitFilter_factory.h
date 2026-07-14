// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, EIC AI background filter proof-of-work

#pragma once

#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/TrackerHitCollection.h>

#include "algorithms/tracking/AiTrackerHitFilter.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class AiTrackerHitFilter_factory
    : public JOmniFactory<AiTrackerHitFilter_factory, AiTrackerHitFilterConfig> {

public:
  using AlgoT = eicrecon::AiTrackerHitFilter;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::TrackerHit> m_all_hits_input{this};
  PodioInput<edm4eic::TrackerHit> m_hits_to_filter_input{this};
  PodioInput<edm4eic::MCRecoTrackerHitAssociation> m_assoc_input{this};

  PodioOutput<edm4eic::TrackerHit> m_signal_output{this};
  PodioOutput<edm4eic::TrackerHit> m_noise_output{this};

  ParameterRef<std::string> m_windowMode{this, "windowMode", config().windowMode,
                                         "t0 source: finder (ONNX CNN) | ideal (MC truth) | off"};
  ParameterRef<std::string> m_finderModelPath{this, "finderModelPath", config().finderModelPath};
  ParameterRef<std::string> m_wattModelPath{this, "wattModelPath", config().wattModelPath};
  ParameterRef<std::string> m_mlpModelPath{this, "mlpModelPath", config().mlpModelPath};
  ParameterRef<float> m_thrIn{this, "thresholdInWindow", config().thresholdInWindow};
  ParameterRef<float> m_thrOut{this, "thresholdOutWindow", config().thresholdOutWindow};
  ParameterRef<float> m_front{this, "windowFrontNs", config().windowFrontNs};
  ParameterRef<float> m_back{this, "windowBackNs", config().windowBackNs};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_all_hits_input(), m_hits_to_filter_input(), m_assoc_input()},
                    {m_signal_output().get(), m_noise_output().get()});
  }
};

} // namespace eicrecon
