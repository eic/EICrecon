// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once




#include <memory>

namespace edm4eic { class ReconstructedParticleCollection; }
namespace edm4hep { class MCParticleCollection; }
namespace spdlog { class logger; }

namespace eicrecon {

    /**
     * Converts edm4hep::MCParticle to edm4eic::ReconstructedParticle
     */
    class MC2SmearedParticle {
    public:

        /** Initialized algorithms with required data. Init function is assumed to be called once **/
        void init(std::shared_ptr<spdlog::logger> logger);

        /** process function convert one data type to another **/
        std::unique_ptr<edm4eic::ReconstructedParticleCollection> produce(const edm4hep::MCParticleCollection* mc_particles);

    private:
        /** algorithm logger */
        std::shared_ptr<spdlog::logger> m_log;
    };
}
