// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim

#pragma once

#include "algorithms/calorimetry/CalorimeterCALOROCReco.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include <edm4hep/SimCalorimeterHitCollection.h>

namespace eicrecon {

class CalorimeterCALOROCReco_factory
    : public JOmniAlgorithmFactory<CalorimeterCALOROCReco_factory, eicrecon::CalorimeterCALOROCReco,
                                   CalorimeterCALOROCRecoConfig> {
private:
  ParameterRef<std::vector<double>> m_attenuationParameters{this, "attenuationParameters",
                                                            config().attenuationParameters};
  ParameterRef<std::vector<double>> m_timeWalkCorrectionParameters{
      this, "timeWalkCorrectionParameters", config().timeWalkCorrectionParameters};
  ParameterRef<std::string> m_attenuationReferencePositionNamePos{
      this, "attenuationReferencePositionNamePos", config().attenuationReferencePositionNamePos};
  ParameterRef<std::string> m_attenuationReferencePositionNameNeg{
      this, "attenuationReferencePositionNameNeg", config().attenuationReferencePositionNameNeg};

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
  ParameterRef<bool> m_useNpeHitPos{this, "useNpeHitPos", config().useNpeHitPos};
};

} // namespace eicrecon
