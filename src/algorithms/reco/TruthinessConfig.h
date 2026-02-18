// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

namespace eicrecon {

struct TruthinessConfig {
  // Penalty weights for different contributions. These scale the relative
  // importance of each term in the overall truthiness score without changing
  // the underlying calculation.

  // Global weight applied to the penalty for mismatched particle identification (PID)
  // between reconstructed and Monte Carlo (MC) particles.
  //
  // - Larger values (> 1) make the truthiness metric more sensitive to PID
  //   mismatches, i.e. a wrong PID assignment will reduce the score more
  //   strongly. This can be useful in studies where PID performance is the
  //   primary focus and you want mis-identified particles to dominate the
  //   metric.
  // - Smaller values (< 1) down-weight PID information relative to other
  //   matching criteria (e.g. kinematics or association completeness). This
  //   can be appropriate if the PID is known to be noisy or still under
  //   development, and you do not want its imperfections to overwhelm the
  //   overall truthiness assessment.
  double pidPenaltyWeight = 1.0;

  // Weight for penalties assigned to MC particles that have no associated
  // reconstructed counterpart.
  //
  // - Increasing this weight emphasizes inefficiencies (missed MC truth
  //   particles) in the reconstruction. This is useful when you want the
  //   metric to strongly penalize missing particles, for example in studies
  //   of reconstruction efficiency.
  // - Decreasing this weight makes the metric more tolerant of unobserved MC
  //   particles, which can be reasonable in regions of phase space where
  //   detector acceptance is limited or where you expect a large amount of
  //   uninstrumented activity.
  double unassociatedMCPenaltyWeight = 1.0;

  // Weight for penalties assigned to reconstructed particles that cannot be
  // matched to any MC particle.
  //
  // - Increasing this weight emphasizes reconstruction purity: extra or fake
  //   reconstructed particles will reduce the truthiness score more strongly.
  //   This is useful when fake-rate control is a priority.
  // - Decreasing this weight makes the metric less sensitive to additional,
  //   possibly spurious, reconstructed objects, which can be acceptable in
  //   high-occupancy environments where a small fake rate is expected.
  double unassociatedRecoPenaltyWeight = 1.0;

  // Default resolutions when the per-particle covariance matrix is unavailable
  // or found to be zero. These act as fallback energy/momentum uncertainties,
  // expressed in GeV, and control how strongly deviations between reco and MC
  // kinematics contribute to the truthiness penalty.

  // Effective energy resolution (sigma_E) used when no reliable energy
  // uncertainty is available from the covariance matrix.
  //
  // - Smaller values (< 1.0 GeV) assume a more precise detector: even small
  //   differences between reconstructed and true energies will be treated as
  //   significant, increasing the sensitivity of the metric to energy
  //   mismatches.
  // - Larger values (> 1.0 GeV) assume a less precise detector: only large
  //   energy differences will be strongly penalized, making the metric less
  //   sensitive to modest energy discrepancies.
  // Adjust this to roughly match the expected energy scale and resolution of
  // the subsystem providing the measurement when no per-object uncertainty is
  // available.
  double defaultEnergyResolution = 1.0;

  // Effective resolution for individual momentum components (px, py, pz), used
  // when the covariance matrix does not provide a usable uncertainty.
  //
  // - Smaller values make the truthiness metric more sensitive to small
  //   differences in the reconstructed vs. true momentum vectors, tightening
  //   the matching requirement.
  // - Larger values make the metric more tolerant to such differences, which
  //   can be appropriate in regions with poorer tracking or for very low/high
  //   momentum particles where the resolution is intrinsically worse.
  double defaultMomentumResolution = 1.0;
};

} // namespace eicrecon
