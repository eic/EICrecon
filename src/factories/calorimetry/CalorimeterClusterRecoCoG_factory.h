// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Chao Peng, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Nathan Brei, Sebouh Paul, Dmitry Kalinkin, Derek Anderson

#pragma once

#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class CalorimeterClusterRecoCoG_factory
    : public JOmniAlgorithmFactory<CalorimeterClusterRecoCoG_factory, CalorimeterClusterRecoCoG> {
  ParameterRef<std::string> m_energyWeight{this, "energyWeight", config().energyWeight};
  ParameterRef<double> m_samplingFraction{this, "samplingFraction", config().sampFrac};
  ParameterRef<double> m_logWeightBase{this, "logWeightBase", config().logWeightBase};
  ParameterRef<std::vector<double>> m_logWeightBaseCoeffs{this, "logWeightBaseCoeffs",
                                                          config().logWeightBaseCoeffs};
  ParameterRef<double> m_logWeightBase_Eref{this, "logWeightBase_Eref",
                                            config().logWeightBase_Eref};
  ParameterRef<bool> m_enableEtaBounds{this, "enableEtaBounds", config().enableEtaBounds};
};

} // namespace eicrecon
