// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim

#pragma once

#include "algorithms/calorimetry/CalorimeterCALOROCCalibration.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterCALOROCCalibration_factory
    : public JOmniFactory<CalorimeterCALOROCCalibration_factory, CalorimeterCALOROCCalibrationConfig> {

private:
public:
  using AlgoT = eicrecon::CalorimeterCALOROCCalibration;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::SimPulse> m_pulseP_input{this};
  PodioInput<edm4eic::RawCALOROCHit> m_CALOROCP_input{this};
  PodioInput<edm4eic::SimPulse> m_pulseN_input{this};
  PodioInput<edm4eic::RawCALOROCHit> m_CALOROCN_input{this};

  PodioOutput<edm4eic::CalorimeterHit> m_rec_hits_output{this};
  PodioOutput<edm4hep::RawCalorimeterHit> m_raw_hits_output{this};
  PodioOutput<edm4eic::MCRecoCalorimeterHitLink> m_raw_link_output{this};
  PodioOutput<edm4eic::MCRecoCalorimeterHitAssociation> m_raw_assoc_output{this};

  ParameterRef<std::vector<double>> m_attenuationParameters{this, "attenuationParameters",
                                                            config().attenuationParameters};
  ParameterRef<std::vector<double>> m_timeWalkCorrectionParameters{this, "timeWalkCorrectionParameters",
                                                            config().timeWalkCorrectionParameters};
  ParameterRef<std::string> m_attenuationReferencePositionNamePos{
      this, "attenuationReferencePositionNamePos", config().attenuationReferencePositionNamePos};
  ParameterRef<std::string> m_attenuationReferencePositionNameNeg{
      this, "attenuationReferencePositionNameNeg", config().attenuationReferencePositionNameNeg};

  ParameterRef<double>      m_slope{this, "slope", config().slope};
  ParameterRef<double>      m_intercept{this, "intercept", config().intercept};
  ParameterRef<uint16_t>    m_highGainDR{this, "highGainDR", config().highGainDR};
  ParameterRef<double>      m_gainRatio{this, "gainRatio", config().gainRatio};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::string> m_layerField{this, "layerField", config().layerField};
  ParameterRef<std::string> m_sectorField{this, "sectorField", config().sectorField};
  ParameterRef<std::string> m_localDetElement{this, "localDetElement", config().localDetElement};
  ParameterRef<std::vector<std::string>> m_localDetFields{this, "localDetFields",
                                                          config().localDetFields};
  ParameterRef<eicrecon::CalorimeterCALOROCCalibrationConfig::ProxyType> m_proxy_type{this, "proxyType",
                                                                              config().proxy_type};
  ParameterRef<bool>        m_timeWalkCor{this, "timeWalkCor", config().timeWalkCor};
  ParameterRef<bool>        m_usePulsePos{this, "usePulsePos", config().usePulsePos};
  ParameterRef<bool>        m_usePulseNPE{this, "usePulseNPE", config().usePulseNPE};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_pulseP_input(), m_CALOROCP_input(), m_pulseN_input(), m_CALOROCN_input()}, 
                    {m_rec_hits_output().get(), m_raw_hits_output().get(), m_raw_link_output().get(), m_raw_assoc_output().get()});
  }
};

} // namespace eicrecon
