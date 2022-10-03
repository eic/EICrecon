// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_PARTICLESWITHASSOCIATION_H
#define EICRECON_PARTICLESWITHASSOCIATION_H

#include <vector>

#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>

namespace eicrecon {

    class ParticlesWithAssociation {
    public:

        explicit ParticlesWithAssociation(std::vector<edm4eic::ReconstructedParticle*> particles,
                                          std::vector<edm4eic::MCRecoParticleAssociation*> associations):
                m_particles(std::move(particles)), m_associations(std::move(associations))
                {}


    private:
        /// Resulting reconstructed particles
        std::vector<edm4eic::ReconstructedParticle*> m_particles;

        /// Resulting associations
        std::vector<edm4eic::MCRecoParticleAssociation*> m_associations;


    };

} // eicrecon

#endif //EICRECON_PARTICLESWITHASSOCIATION_H
