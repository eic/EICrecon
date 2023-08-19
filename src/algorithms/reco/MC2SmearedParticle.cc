// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#include "MC2SmearedParticle.h"

#include <cmath>
#include <edm4eic/vector_utils.h>
#include <edm4eic/ReconstructedParticleCollection.h>


void eicrecon::MC2SmearedParticle::init(std::shared_ptr<spdlog::logger> logger) {
    m_log = logger;
}

std::unique_ptr<edm4eic::ReconstructedParticleCollection> eicrecon::MC2SmearedParticle::produce(const edm4hep::MCParticleCollection* mc_particles) {
    auto rec_particles = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    for (const auto& mc_particle: *mc_particles) {

        if (mc_particle.getGeneratorStatus() != 1) {
            m_log->debug("ignoring particle with generatorStatus = {}", mc_particle.getGeneratorStatus());
            continue;
        }

        // make sure we keep types consistent
        using MomType = decltype(edm4eic::ReconstructedParticle().getMomentum().x);

        const MomType px = mc_particle.getMomentum().x;
        const MomType py = mc_particle.getMomentum().y;
        const MomType pz = mc_particle.getMomentum().z;

        const MomType vx = mc_particle.getVertex().x;
        const MomType vy = mc_particle.getVertex().y;
        const MomType vz = mc_particle.getVertex().z;

        auto rec_part = rec_particles->create();
        rec_part.setType(mc_particle.getGeneratorStatus()); // @TODO: determine type codes
        rec_part.setEnergy(mc_particle.getEnergy());
        rec_part.setMomentum({px, py, pz});
        rec_part.setReferencePoint({vx, vy, vz}); // @FIXME: probably not what we want?
        rec_part.setCharge(mc_particle.getCharge());
        rec_part.setMass(mc_particle.getMass());
        rec_part.setGoodnessOfPID(1); // Perfect PID
        rec_part.setCovMatrix({0, 0, 0, 0});
        rec_part.setPDG(mc_particle.getPDG());
    }
    return std::move(rec_particles);
}
