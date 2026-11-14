// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Alex Jentsch, Jihee Kim, Brian Page
//

#pragma once

#include "algorithms/reco/UndoAfterBurner.h"
#include "algorithms/reco/UndoAfterBurnerConfig.h"

// Event Model related classes
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>

#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class UndoAfterBurnerMCParticles_factory
    : public JOmniAlgorithmFactory<UndoAfterBurnerMCParticles_factory, eicrecon::UndoAfterBurner,
                                   UndoAfterBurnerConfig> {
private:
  ParameterRef<bool> m_pid_assume_pion_mass{this, "m_pid_assume_pion_mass",
                                            config().m_pid_assume_pion_mass};
  ParameterRef<double> m_crossing_angle{this, "m_crossing_angle", config().m_crossing_angle};
  ParameterRef<double> m_pid_purity{this, "m_pid_purity", config().m_pid_purity};
  ParameterRef<bool> m_correct_beam_FX{this, "m_correct_beam_FX", config().m_correct_beam_FX};
  ParameterRef<bool> m_pid_use_MC_truth{this, "m_pid_use_MC_truth", config().m_pid_use_MC_truth};
  ParameterRef<int> m_max_gen_status{this, "m_max_gen_status", config().m_max_gen_status,
                                     "Upper limit on generator status to process (-1 = no limit). "
                                     "Use to filter out background particles and conserve memory."};
};

} // namespace eicrecon
