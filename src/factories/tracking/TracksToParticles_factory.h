// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2024, Dmitry Kalinkin

#pragma once

#include "algorithms/tracking/TracksToParticles.h"
#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/Track.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleID.h>
#include <memory>

namespace eicrecon {

class TracksToParticles_factory
    : public JOmniAlgorithmFactory<TracksToParticles_factory, eicrecon::TracksToParticles,
                                   NoConfig> {};

} // namespace eicrecon
