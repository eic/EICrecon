// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterHitReco.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class CalorimeterHitReco_factory
    : public JOmniAlgorithmFactory<CalorimeterHitReco_factory, eicrecon::CalorimeterHitReco,
                                   CalorimeterHitRecoConfig> {
private:
  ParameterRef<unsigned int> m_capADC{this, "capacityADC", config().capADC};
  ParameterRef<double> m_dyRangeADC{this, "dynamicRangeADC", config().dyRangeADC};
  ParameterRef<unsigned int> m_pedMeanADC{this, "pedestalMean", config().pedMeanADC};
  ParameterRef<double> m_pedSigmaADC{this, "pedestalSigma", config().pedSigmaADC};
  ParameterRef<double> m_resolutionTDC{this, "resolutionTDC", config().resolutionTDC};
  ParameterRef<double> m_thresholdFactor{this, "thresholdFactor", config().thresholdFactor};
  ParameterRef<double> m_thresholdValue{this, "thresholdValue", config().thresholdValue};
  ParameterRef<std::string> m_samplingFraction{this, "samplingFraction", config().sampFrac};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::string> m_layerField{this, "layerField", config().layerField};
  ParameterRef<std::string> m_sectorField{this, "sectorField", config().sectorField};
  ParameterRef<std::string> m_localDetElement{this, "localDetElement", config().localDetElement};
  ParameterRef<std::vector<std::string>> m_localDetFields{this, "localDetFields",
                                                          config().localDetFields};
};

} // namespace eicrecon
