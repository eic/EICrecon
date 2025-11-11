// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Chao Peng, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Nathan Brei, Dmitry Kalinkin

#pragma once

#include "algorithms/calorimetry/CalorimeterHitDigi.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterHitDigi_factory
    : public JOmniFactory<CalorimeterHitDigi_factory, CalorimeterHitDigiConfig> {

public:
  using AlgoT = eicrecon::CalorimeterHitDigi;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::EventHeader> m_event_headers_input{this};
  PodioInput<edm4hep::SimCalorimeterHit> m_hits_input{this};
  PodioOutput<edm4hep::RawCalorimeterHit> m_hits_output{this};
  PodioOutput<edm4eic::MCRecoCalorimeterHitAssociation> m_hit_assocs_output{this};

  ParameterRef<std::vector<double>> m_energyResolutions{this, "energyResolutions", config().eRes};
  ParameterRef<double> m_timeResolution{this, "timeResolution", config().tRes};
  ParameterRef<unsigned int> m_capADC{this, "capacityADC", config().capADC};
  ParameterRef<double> m_dyRangeADC{this, "dynamicRangeADC", config().dyRangeADC};
  ParameterRef<unsigned int> m_pedMeanADC{this, "pedestalMean", config().pedMeanADC};
  ParameterRef<double> m_pedSigmaADC{this, "pedestalSigma", config().pedSigmaADC};
  ParameterRef<double> m_resolutionTDC{this, "resolutionTDC", config().resolutionTDC};
  ParameterRef<std::string> m_corrMeanScale{this, "scaleResponse", config().corrMeanScale};
  ParameterRef<std::vector<std::string>> m_fields{this, "signalSumFields", config().fields};
  ParameterRef<std::string> m_readout{this, "readoutClass", config().readout};
  ParameterRef<std::string> m_readoutType{this, "readoutType", config().readoutType};
  ParameterRef<double> m_lightYield{this, "lightYield", config().lightYield};
  ParameterRef<double> m_photonDetectionEfficiency{this, "photonDetectionEfficiency",
                                                   config().photonDetectionEfficiency};
  ParameterRef<unsigned long long> m_numEffectiveSipmPixels{this, "numEffectiveSipmPixels",
                                                            config().numEffectiveSipmPixels};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_event_headers_input(), m_hits_input()},
                    {m_hits_output().get(), m_hit_assocs_output().get()});
  }
};

} // namespace eicrecon
