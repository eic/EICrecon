// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include "algorithms/calorimetry/CalorimeterCALOROCSingleEndedReco.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CalorimeterCALOROCSingleEndedReco_factory
    : public JOmniFactory<CalorimeterCALOROCSingleEndedReco_factory,
                          CalorimeterCALOROCSingleEndedRecoConfig> {

public:
  using AlgoT = eicrecon::CalorimeterCALOROCSingleEndedReco;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::RawCALOROCHit> m_caloroc_hits_input{this};
  PodioInput<edm4eic::SimPulse> m_pulses_input{this};
  PodioInput<edm4hep::SimCalorimeterHit> m_source_hits_input{this};
  PodioOutput<edm4eic::CalorimeterHit> m_reco_hits_output{this};
  PodioOutput<edm4hep::RawCalorimeterHit> m_hits_output{this};
  PodioOutput<edm4eic::MCRecoCalorimeterHitLink> m_links_output{this};
  PodioOutput<edm4eic::MCRecoCalorimeterHitAssociation> m_hit_assocs_output{this};

  ParameterRef<std::string> m_calorocType{this, "calorocType", config().calorocType};
  ParameterRef<double> m_adcToEnergy{this, "adcToEnergy", config().adcToEnergy};
  ParameterRef<std::vector<double>> m_totEnergyParameters{this, "totEnergyParameters",
                                                          config().totEnergyParameters};
  ParameterRef<double> m_pedestalADC{this, "pedestalADC", config().pedestalADC};
  ParameterRef<bool> m_timeWalkCor{this, "timeWalkCor", config().timeWalkCor};
  ParameterRef<std::vector<double>> m_timeWalkCorrectionParameters{
      this, "timeWalkCorrectionParameters", config().timeWalkCorrectionParameters};
  ParameterRef<std::string> m_samplingFraction{this, "samplingFraction", config().sampFrac};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::string> m_layerField{this, "layerField", config().layerField};

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

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_caloroc_hits_input(), m_pulses_input(), m_source_hits_input()},
                    {m_reco_hits_output().get(), m_hits_output().get(), m_links_output().get(),
                     m_hit_assocs_output().get()});
  }
};

} // namespace eicrecon
