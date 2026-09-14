// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include "algorithms/calorimetry/CALOROCToRawCalorimeterHit.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CALOROCToRawCalorimeterHit_factory
    : public JOmniFactory<CALOROCToRawCalorimeterHit_factory,
                          CALOROCToRawCalorimeterHitConfig> {

public:
  using AlgoT = eicrecon::CALOROCToRawCalorimeterHit;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::RawCALOROCHit> m_caloroc_hits_input{this};
  PodioInput<edm4eic::SimPulse> m_pulses_input{this};
  PodioOutput<edm4hep::RawCalorimeterHit> m_hits_output{this};
  PodioOutput<edm4eic::MCRecoCalorimeterHitLink> m_links_output{this};
  PodioOutput<edm4eic::MCRecoCalorimeterHitAssociation> m_hit_assocs_output{this};

  ParameterRef<std::string> m_calorocType{this, "calorocType", config().calorocType};
  ParameterRef<unsigned int> m_calorocADCSaturation{this, "calorocADCSaturation",
                                                    config().calorocADCSaturation};
  ParameterRef<double> m_calorocResponseToEnergy{this, "calorocResponseToEnergy",
                                                 config().calorocResponseToEnergy};
  ParameterRef<double> m_calorocTOTToEnergy{this, "calorocTOTToEnergy",
                                            config().calorocTOTToEnergy};
  ParameterRef<double> m_calorocTOTOffset{this, "calorocTOTOffset", config().calorocTOTOffset};

  ParameterRef<double> m_time_window{this, "timeWindow", config().caloroc.time_window};
  ParameterRef<unsigned int> m_calorocCapADC{this, "calorocCapADC", config().caloroc.capADC};
  ParameterRef<double> m_dyRangeSingleGainADC{this, "dyRangeSingleGainADC",
                                              config().caloroc.dyRangeSingleGainADC};
  ParameterRef<double> m_dyRangeHighGainADC{this, "dyRangeHighGainADC",
                                            config().caloroc.dyRangeHighGainADC};
  ParameterRef<double> m_dyRangeLowGainADC{this, "dyRangeLowGainADC",
                                           config().caloroc.dyRangeLowGainADC};
  ParameterRef<unsigned int> m_capTOA{this, "capTOA", config().caloroc.capTOA};
  ParameterRef<double> m_dyRangeTOA{this, "dyRangeTOA", config().caloroc.dyRangeTOA};
  ParameterRef<unsigned int> m_capTOT{this, "capTOT", config().caloroc.capTOT};
  ParameterRef<double> m_dyRangeTOT{this, "dyRangeTOT", config().caloroc.dyRangeTOT};

  ParameterRef<unsigned int> m_capADC{this, "capacityADC", config().capADC};
  ParameterRef<double> m_dyRangeADC{this, "dynamicRangeADC", config().dyRangeADC};
  ParameterRef<unsigned int> m_pedMeanADC{this, "pedestalMean", config().pedMeanADC};
  ParameterRef<double> m_resolutionTDC{this, "resolutionTDC", config().resolutionTDC};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_caloroc_hits_input(), m_pulses_input()},
                    {m_hits_output().get(), m_links_output().get(), m_hit_assocs_output().get()});
  }
};

} // namespace eicrecon
