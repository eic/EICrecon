// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#include "MC2SmearedParticle.h"
#include <cmath>
#include <edm4eic/vector_utils.h>
#include <edm4eic/MutableReconstructedParticle.h>


void eicrecon::MC2SmearedParticle::init(std::shared_ptr<spdlog::logger> logger) {
    m_log = logger;

    // Create random gauss function
    m_gauss = [&]() {
        return m_random.Gaus(0, m_cfg.momentum_smearing);
    };
}

edm4eic::ReconstructedParticle *eicrecon::MC2SmearedParticle::produce(const edm4hep::MCParticle *mc_particle) {

    if (mc_particle->getGeneratorStatus() > 1) {
        m_log->debug("ignoring particle with generatorStatus = {}", mc_particle->getGeneratorStatus());
        return nullptr;
    }

    // for now just use total momentum smearing as this is the largest effect,
    // ideally we should also smear the angles but this should be good enough
    // for now.
    // TODO smearing was removed, should we add it again? And is it needed at all?
    const auto energy_true = mc_particle->getEnergy();
    // make sure we keep energy consistent
    using MomType = decltype(edm4eic::ReconstructedParticle().getMomentum().x);

    const MomType px = mc_particle->getMomentum().x;
    const MomType py = mc_particle->getMomentum().y;
    const MomType pz = mc_particle->getMomentum().z;

    const MomType dpx = 0;  // TODO it is 0 as smearing is removed
    const MomType dpy = 0;  // TODO it is 0 as smearing is removed
    const MomType dpz = 0;  // TODO it is 0 as smearing is removed
    const MomType dE =  0;  // TODO it is 0 as smearing is removed

    // ignore covariance for now
    // @TODO: vertex smearing
    const MomType vx = mc_particle->getVertex().x;
    const MomType vy = mc_particle->getVertex().y;
    const MomType vz = mc_particle->getVertex().z;

    edm4eic::MutableReconstructedParticle rec_part;
    rec_part.setType(mc_particle->getGeneratorStatus()); // @TODO: determine type codes
    rec_part.setEnergy(energy_true);
    rec_part.setMomentum({px, py, pz});
    rec_part.setReferencePoint({vx, vy, vz}); // @FIXME: probably not what we want?
    rec_part.setCharge(mc_particle->getCharge());
    rec_part.setMass(mc_particle->getMass());
    rec_part.setGoodnessOfPID(1); // Perfect PID
    rec_part.setCovMatrix({0, 0, 0, 0});
    rec_part.setPDG(mc_particle->getPDG());

    return new edm4eic::ReconstructedParticle(rec_part);

}
