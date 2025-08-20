// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#include "MC2ReconstructedParticle.h"

#include <edm4eic/Cov4f.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <gsl/pointers>

namespace eicrecon {

void MC2ReconstructedParticle::init() {}

void MC2ReconstructedParticle::process(const MC2ReconstructedParticle::Input& input,
                                       const MC2ReconstructedParticle::Output& output) const {

  const auto [mc_particles] = input;
  auto [rec_particles]      = output;

  for (const auto& mc_particle : *mc_particles) {

    if (mc_particle.getGeneratorStatus() != 1) {
      debug("ignoring particle with generatorStatus = {}", mc_particle.getGeneratorStatus());
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
}

} // namespace eicrecon
