// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/InclusiveKinematicsTruthDressed.h"
#include "algorithms/reco/InclusiveKinematicsTruthDressedConfig.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class InclusiveKinematicsTruthDressed_factory
    : public JOmniAlgorithmFactory<InclusiveKinematicsTruthDressed_factory,
                                   eicrecon::InclusiveKinematicsTruthDressed,
                                   InclusiveKinematicsTruthDressedConfig> {
private:
  ParameterRef<double> m_coneDeltaR{this, "coneDeltaR", config().coneDeltaR};
};

} // namespace eicrecon
