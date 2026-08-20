// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#pragma once

#include <edm4eic/ReconstructedParticleCollection.h>
#include <cmath>
#include <string>

namespace eicrecon {

/// Utilities for comparing PandoraPFA and ArborPFA outputs.
/// Provides functions to quantify algorithm performance differences.
class PandoraArborComparison {
public:
  /// Simple metrics comparing two particle collections.
  struct ComparisonMetrics {
    size_t pandora_count     = 0;
    size_t arbor_count       = 0;
    double avg_energy_diff   = 0.0; // (E_arbor - E_pandora) / E_pandora
    double avg_momentum_diff = 0.0;
    int matching_particles   = 0;
  };

  /// Compare PandoraPFA and ArborPFA output collections.
  /// @param pandora_particles   PandoraPFA output
  /// @param arbor_particles     ArborPFA output
  /// @return                    Comparison metrics
  static ComparisonMetrics
  compare(const edm4eic::ReconstructedParticleCollection& pandora_particles,
          const edm4eic::ReconstructedParticleCollection& arbor_particles) {
    ComparisonMetrics metrics;
    metrics.pandora_count = pandora_particles.size();
    metrics.arbor_count   = arbor_particles.size();

    // Simple matching: pair particles by closest energy
    double total_energy_diff   = 0.0;
    double total_momentum_diff = 0.0;
    int matched                = 0;

    for (const auto& p_part : pandora_particles) {
      double p_energy    = p_part.getEnergy();
      double min_delta_e = 1e9;
      int best_idx       = -1;

      for (size_t i = 0; i < arbor_particles.size(); ++i) {
        const auto& a_part = arbor_particles[i];
        double delta_e     = std::abs(a_part.getEnergy() - p_energy);
        if (delta_e < min_delta_e) {
          min_delta_e = delta_e;
          best_idx    = i;
        }
      }

      if (best_idx >= 0 && min_delta_e < p_energy * 0.5) {
        const auto& a_part = arbor_particles[best_idx];
        matched++;
        double energy_diff   = (a_part.getEnergy() - p_energy) / p_energy;
        auto p_mom           = p_part.getMomentum();
        auto a_mom           = a_part.getMomentum();
        double p_mag         = std::hypot({p_mom.x, p_mom.y, p_mom.z});
        double a_mag         = std::hypot({a_mom.x, a_mom.y, a_mom.z});
        double momentum_diff = p_mag > 0 ? (a_mag - p_mag) / p_mag : 0.0;

        total_energy_diff += energy_diff;
        total_momentum_diff += momentum_diff;
      }
    }

    metrics.matching_particles = matched;
    metrics.avg_energy_diff    = matched > 0 ? total_energy_diff / matched : 0.0;
    metrics.avg_momentum_diff  = matched > 0 ? total_momentum_diff / matched : 0.0;

    return metrics;
  }

  /// Print comparison metrics to a string.
  static std::string metricsString(const ComparisonMetrics& metrics) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "PandoraPFA: %zu particles | ArborPFA: %zu particles | "
             "Matched: %d | ΔE/E: %.3f | Δp/p: %.3f",
             metrics.pandora_count, metrics.arbor_count, metrics.matching_particles,
             metrics.avg_energy_diff, metrics.avg_momentum_diff);
    return std::string(buffer);
  }
};

} // namespace eicrecon
