// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "Truthiness.h"

#include <edm4eic/Cov4f.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TruthinessContribution.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <set>
#include <vector>

#include "algorithms/reco/TruthinessConfig.h"

#if __has_include(<edm4eic/Truthiness.h>)
#include <edm4eic/Truthiness.h>
#include <edm4eic/TruthinessCollection.h>
#endif

namespace eicrecon {

void Truthiness::process(const Truthiness::Input& input,
                         [[maybe_unused]] const Truthiness::Output& output) const {

  const auto [mc_particles, rc_particles, associations] = input;

  double truthiness                  = 0.0;
  double total_pid_contribution      = 0.0;
  double total_energy_contribution   = 0.0;
  double total_momentum_contribution = 0.0;

  // Track which MC particles and reconstructed particles are covered by associations
  std::set<edm4hep::MCParticle> associated_mc_particles;
  std::set<edm4eic::ReconstructedParticle> associated_rc_particles;

#if __has_include(<edm4eic/Truthiness.h>)
  // Vectors to store per-association contributions
  std::vector<edm4eic::TruthinessContribution> assoc_truthiness_vec;
  assoc_truthiness_vec.reserve(associations->size());
#endif

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

    // Get covariance matrix elements for uncertainties
    const auto rc_cov = rc_part.getCovMatrix();

    // Energy uncertainty (from time-time covariance tt, or use configured default)
    const bool energy_has_cov = (rc_cov.tt > 0.0);
    const double energy_error =
        energy_has_cov ? std::sqrt(rc_cov.tt) : m_cfg.defaultEnergyResolution;
    if (!energy_has_cov) {
      trace("  Energy uncertainty using default: {:.3f} GeV (covMatrix.tt = {})",
            m_cfg.defaultEnergyResolution, rc_cov.tt);
    }

    // Momentum component uncertainties (from spatial covariance, or use configured default)
    const bool px_has_cov = (rc_cov.xx > 0.0);
    const bool py_has_cov = (rc_cov.yy > 0.0);
    const bool pz_has_cov = (rc_cov.zz > 0.0);
    const double px_error = px_has_cov ? std::sqrt(rc_cov.xx) : m_cfg.defaultMomentumResolution;
    const double py_error = py_has_cov ? std::sqrt(rc_cov.yy) : m_cfg.defaultMomentumResolution;
    const double pz_error = pz_has_cov ? std::sqrt(rc_cov.zz) : m_cfg.defaultMomentumResolution;
    if (!px_has_cov || !py_has_cov || !pz_has_cov) {
      trace("  Momentum uncertainty using default for [{},{},{}]: {:.3f} GeV (covMatrix: xx={}, "
            "yy={}, zz={})",
            px_has_cov ? "cov" : "def", py_has_cov ? "cov" : "def", pz_has_cov ? "cov" : "def",
            m_cfg.defaultMomentumResolution, rc_cov.xx, rc_cov.yy, rc_cov.zz);
    }

    // Calculate energy penalty: sqrt of normalized squared difference
    const double energy_diff    = mc_energy - rc_energy;
    const double energy_chi2    = (energy_diff * energy_diff) / (energy_error * energy_error);
    const double energy_penalty = std::sqrt(energy_chi2);

    // Calculate momentum component penalties: sqrt of sum of normalized squared differences
    const double px_diff          = mc_momentum.x - rc_momentum.x;
    const double py_diff          = mc_momentum.y - rc_momentum.y;
    const double pz_diff          = mc_momentum.z - rc_momentum.z;
    const double px_chi2          = (px_diff * px_diff) / (px_error * px_error);
    const double py_chi2          = (py_diff * py_diff) / (py_error * py_error);
    const double pz_chi2          = (pz_diff * pz_diff) / (pz_error * pz_error);
    const double momentum_chi2    = px_chi2 + py_chi2 + pz_chi2;
    const double momentum_penalty = std::sqrt(momentum_chi2);

    // PDG code mismatch penalty (weighted by configuration)
    const double pdg_penalty_base = (mc_part.getPDG() != rc_part.getPDG()) ? 1.0 : 0.0;
    const double pdg_penalty      = m_cfg.pidPenaltyWeight * pdg_penalty_base;

    const double assoc_penalty = energy_penalty + momentum_penalty + pdg_penalty;

    trace("Association: MC PDG={} (E={:.3f}, p=[{:.3f},{:.3f},{:.3f}]) <-> "
          "RC PDG={} (E={:.3f}±{:.3f}, p=[{:.3f}±{:.3f},{:.3f}±{:.3f},{:.3f}±{:.3f}])",
          mc_part.getPDG(), mc_energy, mc_momentum.x, mc_momentum.y, mc_momentum.z,
          rc_part.getPDG(), rc_energy, energy_error, rc_momentum.x, px_error, rc_momentum.y,
          py_error, rc_momentum.z, pz_error);
    trace("  Energy penalty: {:.6f} (χ²={:.3f}), Momentum penalty: {:.6f} (χ²={:.3f}), PDG "
          "penalty: {:.3f}",
          energy_penalty, energy_chi2, momentum_penalty, momentum_chi2, pdg_penalty);

    truthiness += assoc_penalty;
    total_pid_contribution += pdg_penalty;
    total_energy_contribution += energy_penalty;
    total_momentum_contribution += momentum_penalty;

#if __has_include(<edm4eic/Truthiness.h>)
    assoc_truthiness_vec.push_back({.pid      = static_cast<float>(pdg_penalty),
                                    .energy   = static_cast<float>(energy_penalty),
                                    .momentum = static_cast<float>(momentum_penalty)});
#endif
  }

  // Penalty for unassociated charged MC particles with generator status 1 (final state)
  int unassociated_mc_count = 0;
#if __has_include(<edm4eic/Truthiness.h>)
  std::vector<edm4hep::MCParticle> unassociated_mc_vec;
#endif
  for (const auto& mc_part : *mc_particles) {
    if (mc_part.getGeneratorStatus() == 1 && mc_part.getCharge() != 0.0) {
      if (associated_mc_particles.find(mc_part) == associated_mc_particles.end()) {
        unassociated_mc_count++;
#if __has_include(<edm4eic/Truthiness.h>)
        unassociated_mc_vec.push_back(mc_part);
#endif
        trace("Unassociated MC particle: PDG={}, charge={:.1f}, status={}", mc_part.getPDG(),
              mc_part.getCharge(), mc_part.getGeneratorStatus());
      }
    }
  }
  const double mc_penalty =
      m_cfg.unassociatedMCPenaltyWeight * static_cast<double>(unassociated_mc_count);
  trace("Unassociated charged MC particles (status 1): {} (penalty: {:.2f}, weight: {:.2f})",
        unassociated_mc_count, mc_penalty, m_cfg.unassociatedMCPenaltyWeight);
  truthiness += mc_penalty;

  // Penalty for unassociated reconstructed particles
  int unassociated_rc_count = 0;
#if __has_include(<edm4eic/Truthiness.h>)
  std::vector<edm4eic::ReconstructedParticle> unassociated_rc_vec;
#endif
  for (const auto& rc_part : *rc_particles) {
    if (associated_rc_particles.find(rc_part) == associated_rc_particles.end()) {
      unassociated_rc_count++;
#if __has_include(<edm4eic/Truthiness.h>)
      unassociated_rc_vec.push_back(rc_part);
#endif
      trace("Unassociated reconstructed particle: PDG={}, E={:.3f}, p=[{:.3f},{:.3f},{:.3f}]",
            rc_part.getPDG(), rc_part.getEnergy(), rc_part.getMomentum().x, rc_part.getMomentum().y,
            rc_part.getMomentum().z);
    }
  }
  const double rc_penalty =
      m_cfg.unassociatedRecoPenaltyWeight * static_cast<double>(unassociated_rc_count);
  trace("Unassociated reconstructed particles: {} (penalty: {:.2f}, weight: {:.2f})",
        unassociated_rc_count, rc_penalty, m_cfg.unassociatedRecoPenaltyWeight);
  truthiness += rc_penalty;

  // Update statistics using online updating formula
  // avg_n = avg_(n-1) + (x_n - avg_(n-1)) / n
  {
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    const auto event_count = ++m_event_count;
    m_average_truthiness += (truthiness - m_average_truthiness) / event_count;
  }

  // Report final truthiness
  debug("Event truthiness: {:.6f} (from {} associations, {} unassociated MC, {} unassociated RC)",
        truthiness, associations->size(), unassociated_mc_count, unassociated_rc_count);
  trace("  Total PID contribution: {:.6f}", total_pid_contribution);
  trace("  Total energy contribution: {:.6f}", total_energy_contribution);
  trace("  Total momentum contribution: {:.6f}", total_momentum_contribution);

#if __has_include(<edm4eic/Truthiness.h>)
  // Create output collection if available
  const auto [truthiness_output] = output;
  auto truthiness_obj            = truthiness_output->create();

  // Set scalar values
  truthiness_obj.setTruthiness(static_cast<float>(truthiness));
  truthiness_obj.setAssociationContribution(
      {.pid      = static_cast<float>(total_pid_contribution),
       .energy   = static_cast<float>(total_energy_contribution),
       .momentum = static_cast<float>(total_momentum_contribution)});
  truthiness_obj.setUnassociatedMCParticlesContribution(static_cast<float>(mc_penalty));
  truthiness_obj.setUnassociatedRecoParticlesContribution(static_cast<float>(rc_penalty));

  // Add associations and their contributions
  for (const auto& assoc : *associations) {
    truthiness_obj.addToAssociations(assoc);
  }
  for (const auto& val : assoc_truthiness_vec) {
    truthiness_obj.addToAssociationContributions(val);
  }

  // Add unassociated particles
  for (const auto& mc_part : unassociated_mc_vec) {
    truthiness_obj.addToUnassociatedMCParticles(mc_part);
  }
  for (const auto& rc_part : unassociated_rc_vec) {
    truthiness_obj.addToUnassociatedRecoParticles(rc_part);
  }
#endif
}

} // namespace eicrecon
