// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2024, Dmitry Kalinkin

#pragma once

#include "algorithms/pid/MatchToRICHPID.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include <edm4eic/CherenkovParticleID.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4hep/ParticleID.h>
#include <memory>

namespace eicrecon {

class MatchToRICHPID_factory
    : public JOmniAlgorithmFactory<MatchToRICHPID_factory, eicrecon::MatchToRICHPID,
                                   MatchToRICHPIDConfig> {};

} // namespace eicrecon
