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
    const auto pvec = mc_particle->getMomentum();
    const auto pgen = std::hypot(pvec.x, pvec.y, pvec.z);
    const auto momentum = pgen * m_gauss();
    // make sure we keep energy consistent
    using MomType = decltype(edm4eic::ReconstructedParticle().getMomentum().x);
    const MomType energy =
            std::sqrt(mc_particle->getEnergy() * mc_particle->getEnergy() - pgen * pgen + momentum * momentum);
    const MomType px = mc_particle->getMomentum().x * momentum / pgen;
    const MomType py = mc_particle->getMomentum().y * momentum / pgen;
    const MomType pz = mc_particle->getMomentum().z * momentum / pgen;

    const MomType dpx = m_cfg.momentum_smearing * px;
    const MomType dpy = m_cfg.momentum_smearing * py;
    const MomType dpz = m_cfg.momentum_smearing * pz;
    const MomType dE = m_cfg.momentum_smearing * energy;
    // ignore covariance for now
    // @TODO: vertex smearing
    const MomType vx = mc_particle->getVertex().x;
    const MomType vy = mc_particle->getVertex().y;
    const MomType vz = mc_particle->getVertex().z;

    edm4eic::MutableReconstructedParticle rec_part;
    rec_part.setType(-1); // @TODO: determine type codes
    rec_part.setEnergy(energy);
    rec_part.setMomentum({px, py, pz});
    rec_part.setReferencePoint({vx, vy, vz}); // @FIXME: probably not what we want?
    rec_part.setCharge(mc_particle->getCharge());
    rec_part.setMass(mc_particle->getMass());
    rec_part.setGoodnessOfPID(1); // Perfect PID
    rec_part.setCovMatrix({dpx, dpy, dpz, dE});
    rec_part.setPDG(mc_particle->getPDG());

    return new edm4eic::ReconstructedParticle(rec_part);

}







