// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim, Aiden Wu

#pragma once

#include "algorithms/calorimetry/CalorimeterCALOROCReco.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <stdexcept>

namespace eicrecon {

class CalorimeterCALOROCReco_factory
    : public JOmniFactory<CalorimeterCALOROCReco_factory, CalorimeterCALOROCRecoConfig> {

private:
public:
  using AlgoT = eicrecon::CalorimeterCALOROCReco;

private:
  std::unique_ptr<AlgoT> m_algo;

  // Inputs: source/NPE hits, digitized hits, then pulses for ADC calibration.
  PodioInput<edm4hep::SimCalorimeterHit> m_source_hits_input{this};
  PodioInput<edm4eic::RawCALOROCHit> m_caloroc_hits_input{this};
  VariadicPodioInput<edm4eic::SimPulse> m_pulses_input{this};

  PodioOutput<edm4eic::CalorimeterHit> m_rec_hits_output{this};
  PodioOutput<edm4hep::RawCalorimeterHit> m_raw_hits_output{this};
  VariadicPodioOutput<edm4eic::MCRecoCalorimeterHitLink> m_links_output{this};
  VariadicPodioOutput<edm4eic::MCRecoCalorimeterHitAssociation> m_associations_output{this};

  ParameterRef<std::string> m_calorocType{this, "calorocType", config().calorocType};
  ParameterRef<double> m_adcToEnergy{this, "adcToEnergy", config().adcToEnergy};
  ParameterRef<std::vector<double>> m_totEnergyParameters{this, "totEnergyParameters",
                                                          config().totEnergyParameters};
  ParameterRef<double> m_pedestalADC{this, "pedestalADC", config().pedestalADC};
  ParameterRef<std::string> m_samplingFraction{this, "samplingFraction", config().sampFrac};
  ParameterRef<unsigned int> m_calorocCapADC{this, "calorocCapADC", config().caloroc.capADC};
  ParameterRef<unsigned int> m_capTOT{this, "capTOT", config().caloroc.capTOT};
  ParameterRef<double> m_dyRangeTOT{this, "dyRangeTOT", config().caloroc.dyRangeTOT};

  ParameterRef<std::vector<double>> m_timeWalkCorrectionParameters{
      this, "timeWalkCorrectionParameters", config().timeWalkCorrectionParameters};

  ParameterRef<double> m_time_window{this, "timeWindow", config().caloroc.time_window};
  ParameterRef<unsigned int> m_capTOA{this, "capTOA", config().caloroc.capTOA};
  ParameterRef<double> m_dyRangeTOA{this, "dyRangeTOA", config().caloroc.dyRangeTOA};

  ParameterRef<double> m_slope{this, "slope", config().slope};
  ParameterRef<double> m_intercept{this, "intercept", config().intercept};
  ParameterRef<uint16_t> m_highGainDR{this, "highGainDR", config().highGainDR};
  ParameterRef<double> m_gainRatio{this, "gainRatio", config().gainRatio};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::string> m_layerField{this, "layerField", config().layerField};
  ParameterRef<std::string> m_sectorField{this, "sectorField", config().sectorField};
  ParameterRef<std::string> m_localDetElement{this, "localDetElement", config().localDetElement};
  ParameterRef<std::vector<std::string>> m_localDetFields{this, "localDetFields",
                                                          config().localDetFields};
  ParameterRef<eicrecon::CalorimeterCALOROCRecoConfig::ProxyType> m_proxy_type{this, "proxyType",
                                                                               config().proxy_type};
  ParameterRef<bool> m_timeWalkCor{this, "timeWalkCor", config().timeWalkCor};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    if (config().energyCalibration == CalorimeterCALOROCRecoConfig::EnergyCalibration::ADC &&
        (m_pulses_input.collection_names.size() != 1 ||
         m_links_output.collection_names.size() != 1 ||
         m_associations_output.collection_names.size() != 1)) {
      throw std::runtime_error(
          "ADC reconstruction requires one pulse input and both truth outputs");
    }
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    if (config().energyCalibration == CalorimeterCALOROCRecoConfig::EnergyCalibration::ADC) {
      m_algo->process({m_caloroc_hits_input(), m_pulses_input()[0], m_source_hits_input()},
                      {m_rec_hits_output().get(), m_raw_hits_output().get(),
                       m_links_output()[0].get(), m_associations_output()[0].get()});
    } else {
      m_algo->process({m_source_hits_input(), m_caloroc_hits_input()},
                      {m_rec_hits_output().get(), m_raw_hits_output().get()});
    }
  }
};

} // namespace eicrecon
