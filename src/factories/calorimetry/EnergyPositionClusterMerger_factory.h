// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/calorimetry/EnergyPositionClusterMerger.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class EnergyPositionClusterMerger_factory
    : public JOmniAlgorithmFactory<EnergyPositionClusterMerger_factory,
                                   eicrecon::EnergyPositionClusterMerger,
                                   EnergyPositionClusterMergerConfig> {
private:
  ParameterRef<double> m_energyRelTolerance{this, "energyRelTolerance",
                                            config().energyRelTolerance};
  ParameterRef<double> m_phiTolerance{this, "phiTolerance", config().phiTolerance};
  ParameterRef<double> m_etaTolerance{this, "etaTolerance", config().etaTolerance};
};

} // namespace eicrecon
