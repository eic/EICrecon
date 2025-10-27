// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <edm4eic/EDM4eicVersion.h>

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

  PodioInput<edm4hep::SimCalorimeterHit> m_hits_input{this};
  PodioOutput<edm4hep::RawCalorimeterHit> m_hits_output{this};
#if EDM4EIC_VERSION_MAJOR >= 7
  PodioOutput<edm4eic::MCRecoCalorimeterHitAssociation> m_hit_assocs_output{this};
#endif

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

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
#if EDM4EIC_VERSION_MAJOR >= 7
    m_algo->process({m_hits_input()}, {m_hits_output().get(), m_hit_assocs_output().get()});
#else
    m_algo->process({m_hits_input()}, {m_hits_output().get()});
#endif
  }
};

} // namespace eicrecon
