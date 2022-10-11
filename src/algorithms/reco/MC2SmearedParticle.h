// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#ifndef EICRECON_MC2SMEAREDPARTICLE_H
#define EICRECON_MC2SMEAREDPARTICLE_H

#include <vector>
#include <TRandomGen.h>
#include <spdlog/spdlog.h>

#include <algorithms/interfaces/WithPodConfig.h>
#include <algorithms/interfaces/IObjectProducer.h>

#include <edm4eic/ReconstructedParticle.h>
#include <edm4hep/MCParticle.h>

#include <MC2SmearedParticleConfig.h>


namespace eicrecon {

    /**
     * Converts edm4hep::MCParticle to edm4eic::ReconstructedParticle
     */
    class MC2SmearedParticle:
            public IObjectProducer<edm4hep::MCParticle, edm4eic::ReconstructedParticle>,
            public WithPodConfig<MC2SmearedParticleConfig> {
    public:

        /** Initialized algorithms with required data. Init function is assumed to be called once **/
        void init(std::shared_ptr<spdlog::logger> logger);

        /** process function convert one data type to another **/
        virtual edm4eic::ReconstructedParticle * process(const edm4hep::MCParticle * mc_particle);

    private:
        /** algorithm logger */
        std::shared_ptr<spdlog::logger> m_log;

        /** Random number generation*/
        TRandomMixMax m_random;
        std::function<double()> m_gauss;
    };
}
#endif //EICRECON_MC2SMEAREDPARTICLE_H
