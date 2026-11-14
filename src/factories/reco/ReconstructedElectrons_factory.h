// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Daniel Brandenburg, Wouter Deconinck

#pragma once

#include "extensions/jana/JOmniAlgorithmFactory.h"

#include "algorithms/reco/ElectronReconstruction.h"

namespace eicrecon {

class ReconstructedElectrons_factory
    : public JOmniAlgorithmFactory<ReconstructedElectrons_factory, eicrecon::ElectronReconstruction,
                                   ElectronReconstructionConfig> {
private:
  // Declare parameters
  ParameterRef<double> m_min_energy_over_momentum{this, "minEnergyOverMomentum",
                                                  config().min_energy_over_momentum};
  ParameterRef<double> m_max_energy_over_momentum{this, "maxEnergyOverMomentum",
                                                  config().max_energy_over_momentum};
};

} // namespace eicrecon
