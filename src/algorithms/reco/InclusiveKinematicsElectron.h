// Original header license: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Dmitry Romanov

// Takes a list of particles (presumed to be from tracking), and all available clusters.
// 1. Match clusters to their tracks using the mcID field
// 2. For unmatched clusters create neutrals and add to the particle list

#ifndef EICRECON_INCLUSIVEKINEMATICELECTRON_H
#define EICRECON_INCLUSIVEKINEMATICELECTRON_H

#include <algorithm>
#include <cmath>
#include <vector>
#include <map>

#include <spdlog/spdlog.h>


// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/MCRecoParticleAssociationCollection.h"
#include "edm4eic/ReconstructedParticleCollection.h"
#include "edm4eic/InclusiveKinematicsCollection.h"

#include "ParticlesWithAssociation.h"

namespace eicrecon {

    class InclusiveKinematicsElectron {

    public:

        void init(std::shared_ptr<spdlog::logger> logger);

        ParticlesWithAssociation *execute(
                std::vector<const edm4hep::MCParticle *> mcparticles,
                std::vector<edm4eic::ReconstructedParticle *> inparts,
                std::vector<edm4eic::MCRecoParticleAssociation *> inpartsassoc,
                std::vector<edm4eic::InclusiveKinematicsCollection *> outputInclusiveKinematics);

    private:
        std::shared_ptr<spdlog::logger> m_log;
        
    };

} // namespace eicrecon

#endif //EICRECON_INCLUSIVEKINEMATICELECTRON_H
