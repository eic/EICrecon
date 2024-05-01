// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Daniel Brandenburg
#include "ElectronReconstruction.h"

#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>

#include "algorithms/reco/ElectronReconstructionConfig.h"

namespace eicrecon {

    void ElectronReconstruction::init(std::shared_ptr<spdlog::logger> logger) {
        m_log = logger;
    }

    std::unique_ptr<edm4eic::ReconstructedParticleCollection> ElectronReconstruction::execute(
            const edm4eic::ReconstructedParticleCollection *rcparts
            ) {

        // Some obvious improvements:
        // - E/p cut from real study optimized for electron finding and hadron rejection
        // - use of any HCAL info?
        // - check for duplicates?

        // output container
        auto out_electrons = std::make_unique<edm4eic::ReconstructedParticleCollection>();
        out_electrons->setSubsetCollection(); // out_electrons is a subset of the ReconstructedParticles collection

        for (const auto particle : *rcparts) {
            // if we found a reco particle then test for electron compatibility
            if (particle.getClusters().size() == 0) {
                continue;
            }
            if (particle.getCharge() == 0) { // Skip over photons/other particles without a track
                continue;
            }
            double E = particle.getClusters()[0].getEnergy();
            double p = edm4hep::utils::magnitude(particle.getMomentum());
            double EOverP = E / p;

            m_log->trace("ReconstructedElectron: Energy={} GeV, p={} GeV, E/p = {} for PDG (from truth): {}", E, p, EOverP, particle.getPDG());
            // Apply the E/p cut here to select electons
            if (EOverP >= m_cfg.min_energy_over_momentum && EOverP <= m_cfg.max_energy_over_momentum) {
                out_electrons->push_back(particle);
            }

        }
    m_log->debug("Found {} electron candidates", out_electrons->size());
    return out_electrons;
    }
}
