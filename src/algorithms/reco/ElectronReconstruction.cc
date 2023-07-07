// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Daniel Brandenburg

#include <algorithm>
#include <cmath>
#include <vector>

#include "Beam.h"
#include "Boost.h"
#include "ElectronReconstruction.h"

// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/MCRecoParticleAssociationCollection.h"
#include "edm4eic/ReconstructedParticleCollection.h"

namespace eicrecon {

  void ElectronReconstruction::init(std::shared_ptr<spdlog::logger> logger) {
    m_log = logger;
  }

  std::vector<edm4eic::ReconstructedParticle*> ElectronReconstruction::execute(
    std::vector<const edm4hep::MCParticle *> mcparts,
    std::vector<const edm4eic::ReconstructedParticle *> rcparts,
    std::vector<const edm4eic::MCRecoParticleAssociation *> rcassoc,
    std::vector<std::vector<const edm4eic::MCRecoClusterParticleAssociation*>> &in_clu_assoc
    ) {

        // Step 1. Loop through MCParticle - cluster associations
        // Step 2. Get Reco particle for the Mc Particle matched to cluster
        // Step 3. Apply E/p cut using Reco cluster Energy and Reco Particle momentum

        // Some obvious improvements:
        // - E/p cut from real study optimized for electron finding and hadron rejection
        // - use of any HCAL info?
        // - check for duplicates?

        // output container
        std::vector<edm4eic::ReconstructedParticle*> electrons_edm;

        for ( auto col : in_clu_assoc ){ // loop on cluster association collections
          for ( auto clu_assoc : col ){ // loop on MCRecoClusterParticleAssociation in this particular collection
            auto sim = clu_assoc->getSim(); // McParticle
            auto clu = clu_assoc->getRec(); // RecoCluster

            m_log->debug( "SimId={}, CluId={}", clu_assoc->getSimID(), clu_assoc->getRecID() );
            m_log->debug( "MCParticle: Energy={}, p={}, E/p = {} for PDG: {}", clu.getEnergy(), edm4eic::magnitude(sim.getMomentum()), clu.getEnergy() / edm4eic::magnitude(sim.getMomentum()), sim.getPDG() );


            // Find the Reconstructed particle associated to the MC Particle that is matched with this reco cluster
            // i.e. take (MC Particle <-> RC Cluster) + ( MC Particle <-> RC Particle ) = ( RC Particle <-> RC Cluster )
            auto reco_part_assoc = rcassoc.begin();
            bool found_reco_part = false;
            for (; reco_part_assoc != rcassoc.end(); ++reco_part_assoc) {
              if ((*reco_part_assoc)->getSimID() == (unsigned) clu_assoc->getSimID()) {
                found_reco_part = true;
                break;
              }
            }

            // if we found a reco particle then test for electron compatibility
            if ( found_reco_part ){
              auto reco_part = (*reco_part_assoc)->getRec();
              double EoverP = clu.getEnergy() / edm4eic::magnitude(reco_part.getMomentum());
              m_log->info( "ReconstructedParticle: Energy={}, p={}, E/p = {} for PDG (from truth): {}", clu.getEnergy(), edm4eic::magnitude(reco_part.getMomentum()), EoverP, sim.getPDG() );

              // Apply the E/p cut here to select electons
              if ( EoverP >= min_energy_over_momentum && EoverP <= max_energy_over_momentum ) {
                // doing this doesnt cause an error but resulted in an empty podio output collection
                // electrons_edm.push_back( new edm4eic::ReconstructedParticle( reco_part ) );

                edm4eic::MutableReconstructedParticle electron_edm = reco_part.clone();
                electron_edm.setMass(m_electron);
                electrons_edm.push_back(new edm4eic::ReconstructedParticle(electron_edm));
              }

            } else {
              m_log->debug( "Could not find reconstructed particle for SimId={}", clu_assoc->getSimID() );
            }

          } // loop on MC particle to cluster associations in collection
        } // loop on collections

        m_log->debug( "Found {} electron candidates", electrons_edm.size() );
        return electrons_edm;
    }

}
