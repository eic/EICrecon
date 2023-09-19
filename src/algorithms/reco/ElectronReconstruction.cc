// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Daniel Brandenburg
#include "ElectronReconstruction.h"

#include <edm4eic/Cluster.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/MutableReconstructedParticle.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/vector_utils_legacy.h>
#include <edm4hep/MCParticle.h>
#include <fmt/core.h>
#include <spdlog/logger.h>
#include <exception>

namespace edm4hep { class MCParticleCollection; }

namespace eicrecon {

  void ElectronReconstruction::init(std::shared_ptr<spdlog::logger> logger) {
    m_log = logger;
  }

  std::unique_ptr<edm4eic::ReconstructedParticleCollection> ElectronReconstruction::execute(
    const edm4hep::MCParticleCollection *mcparts,
    const edm4eic::ReconstructedParticleCollection *rcparts,
    const edm4eic::MCRecoParticleAssociationCollection *rcassoc,
    const std::vector<const edm4eic::MCRecoClusterParticleAssociationCollection*> &in_clu_assoc
    ) {

        // Step 1. Loop through MCParticle - cluster associations
        // Step 2. Get Reco particle for the Mc Particle matched to cluster
        // Step 3. Apply E/p cut using Reco cluster Energy and Reco Particle momentum

        // Some obvious improvements:
        // - E/p cut from real study optimized for electron finding and hadron rejection
        // - use of any HCAL info?
        // - check for duplicates?

        // output container
        auto out_electrons = std::make_unique<edm4eic::ReconstructedParticleCollection>();

        for ( const auto *col : in_clu_assoc ){ // loop on cluster association collections
          for ( auto clu_assoc : (*col) ){ // loop on MCRecoClusterParticleAssociation in this particular collection
            auto sim = clu_assoc.getSim(); // McParticle
            auto clu = clu_assoc.getRec(); // RecoCluster

            m_log->trace( "SimId={}, CluId={}", clu_assoc.getSimID(), clu_assoc.getRecID() );
            m_log->trace( "MCParticle: Energy={} GeV, p={} GeV, E/p = {} for PDG: {}", clu.getEnergy(), edm4eic::magnitude(sim.getMomentum()), clu.getEnergy() / edm4eic::magnitude(sim.getMomentum()), sim.getPDG() );


            // Find the Reconstructed particle associated to the MC Particle that is matched with this reco cluster
            // i.e. take (MC Particle <-> RC Cluster) + ( MC Particle <-> RC Particle ) = ( RC Particle <-> RC Cluster )
            auto reco_part_assoc = rcassoc->begin();
            for (; reco_part_assoc != rcassoc->end(); ++reco_part_assoc) {
              if (reco_part_assoc->getSimID() == (unsigned) clu_assoc.getSimID()) {
                break;
              }
            }

            // if we found a reco particle then test for electron compatibility
            if ( reco_part_assoc != rcassoc->end() ){
              auto reco_part = reco_part_assoc->getRec();
              double EoverP = clu.getEnergy() / edm4eic::magnitude(reco_part.getMomentum());
              m_log->trace( "ReconstructedParticle: Energy={} GeV, p={} GeV, E/p = {} for PDG (from truth): {}", clu.getEnergy(), edm4eic::magnitude(reco_part.getMomentum()), EoverP, sim.getPDG() );

              // Apply the E/p cut here to select electons
              if ( EoverP >= min_energy_over_momentum && EoverP <= max_energy_over_momentum ) {
                out_electrons->push_back(reco_part.clone());
              }

            } else {
              m_log->debug( "Could not find reconstructed particle for SimId={}", clu_assoc.getSimID() );
            }

          } // loop on MC particle to cluster associations in collection
        } // loop on collections

        m_log->debug( "Found {} electron candidates", out_electrons->size() );
        return out_electrons;
    }

}
