// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025 Daniel Brandenburg, Wouter Deconinck
#include "ElectronReconstruction.h"

#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/utils/vector_utils.h>
#include <podio/RelationRange.h>
#include <gsl/pointers>

#include "algorithms/reco/ElectronReconstructionConfig.h"

namespace eicrecon {

void ElectronReconstruction::process(const Input& input, const Output& output) const {

  const auto [rcparts] = input;
  auto [out_electrons] = output;

  // Some obvious improvements:
  // - E/p cut from real study optimized for electron finding and hadron rejection
  // - use of any HCAL info?
  // - check for duplicates?

  // output container
  out_electrons
      ->setSubsetCollection(); // out_electrons is a subset of the ReconstructedParticles collection

  for (const auto particle : *rcparts) {
    // if we found a reco particle then test for electron compatibility
    if (particle.getClusters().empty()) {
      continue;
    }
    if (particle.getCharge() == 0) { // Skip over photons/other particles without a track
      continue;
    }
    double E      = particle.getClusters()[0].getEnergy();
    double p      = edm4hep::utils::magnitude(particle.getMomentum());
    double EOverP = E / p;

    trace("ReconstructedElectron: Energy={} GeV, p={} GeV, E/p = {} for PDG (from truth): {}", E, p,
          EOverP, particle.getPDG());
    // Apply the E/p cut here to select electons
    if (EOverP >= m_cfg.min_energy_over_momentum && EOverP <= m_cfg.max_energy_over_momentum) {
      out_electrons->push_back(particle);
    }
  }
  debug("Found {} electron candidates", out_electrons->size());
}
} // namespace eicrecon
