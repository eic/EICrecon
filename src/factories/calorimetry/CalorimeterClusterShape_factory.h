// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/calorimetry/CalorimeterClusterShape.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class CalorimeterClusterShape_factory
    : public JOmniAlgorithmFactory<CalorimeterClusterShape_factory,
                                   eicrecon::CalorimeterClusterShape,
                                   CalorimeterClusterShapeConfig> {
private:
  // parameter bindings
  ParameterRef<bool> m_longitudinalShowerInfoAvailable{this, "longitudinalShowerInfoAvailable",
                                                       config().longitudinalShowerInfoAvailable};
  ParameterRef<std::string> m_energyWeight{this, "energyWeight", config().energyWeight};
  ParameterRef<double> m_sampFrac{this, "sampFrac", config().sampFrac};
  ParameterRef<std::vector<double>> m_logWeightBaseCoeffs{this, "logWeightBaseCoeffs",
                                                          config().logWeightBaseCoeffs};
  ParameterRef<double> m_logWeightBase_Eref{this, "logWeightBase_Eref",
                                            config().logWeightBase_Eref};
  ParameterRef<double> m_logWeightBase{this, "logWeightBase", config().logWeightBase};
}; // end CalorimeterClusterShape_factory

} // namespace eicrecon
