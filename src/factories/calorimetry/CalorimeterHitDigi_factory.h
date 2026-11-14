// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Chao Peng, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Nathan Brei, Dmitry Kalinkin

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/calorimetry/CalorimeterHitDigi.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class CalorimeterHitDigi_factory
    : public JOmniAlgorithmFactory<CalorimeterHitDigi_factory, eicrecon::CalorimeterHitDigi,
                                   CalorimeterHitDigiConfig> {
private:
  ParameterRef<std::vector<double>> m_energyResolutions{this, "energyResolutions", config().eRes};
  ParameterRef<double> m_timeResolution{this, "timeResolution", config().tRes};
  ParameterRef<unsigned int> m_capADC{this, "capacityADC", config().capADC};
  ParameterRef<double> m_dyRangeADC{this, "dynamicRangeADC", config().dyRangeADC};
  ParameterRef<unsigned int> m_pedMeanADC{this, "pedestalMean", config().pedMeanADC};
  ParameterRef<double> m_pedSigmaADC{this, "pedestalSigma", config().pedSigmaADC};
  ParameterRef<double> m_resolutionTDC{this, "resolutionTDC", config().resolutionTDC};
  ParameterRef<double> m_integrationWindow{this, "integrationWindow", config().integrationWindow};
  ParameterRef<std::string> m_corrMeanScale{this, "scaleResponse", config().corrMeanScale};
  ParameterRef<std::vector<std::string>> m_fields{this, "signalSumFields", config().fields};
  ParameterRef<std::string> m_readout{this, "readoutClass", config().readout};
  ParameterRef<std::string> m_readoutType{this, "readoutType", config().readoutType};
  ParameterRef<double> m_lightYield{this, "lightYield", config().lightYield};
  ParameterRef<double> m_photonDetectionEfficiency{this, "photonDetectionEfficiency",
                                                   config().photonDetectionEfficiency};
  ParameterRef<unsigned long long> m_numEffectiveSipmPixels{this, "numEffectiveSipmPixels",
                                                            config().numEffectiveSipmPixels};
};

} // namespace eicrecon
