// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "Truthiness.h"

#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <map>
#include <set>

namespace eicrecon {

void Truthiness::process(const Truthiness::Input& input,
                         const Truthiness::Output& /* output */) const {

  const auto [mc_particles, rc_particles, associations] = input;

  double truthiness = 0.0;

  // Track which MC particles and reconstructed particles are covered by associations
  std::set<edm4hep::MCParticle> associated_mc_particles;
  std::set<edm4eic::ReconstructedParticle> associated_rc_particles;

  // Process all associations
  for (const auto& assoc : *associations) {
    const auto& mc_part = assoc.getSim();
    const auto& rc_part = assoc.getRec();

    // Track that these particles have associations
    associated_mc_particles.insert(mc_part);
    associated_rc_particles.insert(rc_part);

    // Get particle properties
    const auto mc_momentum = mc_part.getMomentum();
    const auto rc_momentum = rc_part.getMomentum();

    const double mc_p      = edm4hep::utils::magnitude(mc_momentum);
    const double mc_energy = std::sqrt(mc_p * mc_p + mc_part.getMass() * mc_part.getMass());
    const double rc_energy = rc_part.getEnergy();

    // Calculate energy difference squared
    const double energy_diff    = mc_energy - rc_energy;
    const double energy_penalty = energy_diff * energy_diff;

    // Calculate momentum component differences squared
    const double px_diff          = mc_momentum.x - rc_momentum.x;
    const double py_diff          = mc_momentum.y - rc_momentum.y;
    const double pz_diff          = mc_momentum.z - rc_momentum.z;
    const double momentum_penalty = px_diff * px_diff + py_diff * py_diff + pz_diff * pz_diff;

    // PDG code mismatch penalty
    const double pdg_penalty = (mc_part.getPDG() != rc_part.getPDG()) ? 1.0 : 0.0;

    const double assoc_penalty = energy_penalty + momentum_penalty + pdg_penalty;

    debug("Association: MC PDG={} (E={:.3f}, p=[{:.3f},{:.3f},{:.3f}]) <-> "
          "RC PDG={} (E={:.3f}, p=[{:.3f},{:.3f},{:.3f}])",
          mc_part.getPDG(), mc_energy, mc_momentum.x, mc_momentum.y, mc_momentum.z,
          rc_part.getPDG(), rc_energy, rc_momentum.x, rc_momentum.y, rc_momentum.z);
    debug("  Energy penalty: {:.6f}, Momentum penalty: {:.6f}, PDG penalty: {:.0f}", energy_penalty,
          momentum_penalty, pdg_penalty);

    truthiness += assoc_penalty;
  }

  // Penalty for unassociated charged MC particles with generator status 2
  int unassociated_mc_count = 0;
  for (const auto& mc_part : *mc_particles) {
    if (mc_part.getGeneratorStatus() == 2 && mc_part.getCharge() != 0.0) {
      if (associated_mc_particles.find(mc_part) == associated_mc_particles.end()) {
        unassociated_mc_count++;
        debug("Unassociated MC particle: PDG={}, charge={:.1f}, status={}", mc_part.getPDG(),
              mc_part.getCharge(), mc_part.getGeneratorStatus());
      }
    }
  }
  const double mc_penalty = static_cast<double>(unassociated_mc_count);
  debug("Unassociated charged MC particles (status 2): {} (penalty: {:.0f})", unassociated_mc_count,
        mc_penalty);
  truthiness += mc_penalty;

  // Penalty for unassociated reconstructed particles
  int unassociated_rc_count = 0;
  for (const auto& rc_part : *rc_particles) {
    if (associated_rc_particles.find(rc_part) == associated_rc_particles.end()) {
      unassociated_rc_count++;
      debug("Unassociated reconstructed particle: PDG={}, E={:.3f}, p=[{:.3f},{:.3f},{:.3f}]",
            rc_part.getPDG(), rc_part.getEnergy(), rc_part.getMomentum().x, rc_part.getMomentum().y,
            rc_part.getMomentum().z);
    }
  }
  const double rc_penalty = static_cast<double>(unassociated_rc_count);
  debug("Unassociated reconstructed particles: {} (penalty: {:.0f})", unassociated_rc_count,
        rc_penalty);
  truthiness += rc_penalty;

  // Report final truthiness
  info("Event truthiness: {:.6f} (from {} associations, {} unassociated MC, {} unassociated RC)",
       truthiness, associations->size(), unassociated_mc_count, unassociated_rc_count);
}

} // namespace eicrecon
