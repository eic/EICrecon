// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

using InclusiveKinematicsTruthBornAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::MCParticleCollection>,
                          algorithms::Output<edm4eic::InclusiveKinematicsCollection>>;

/// Born-level inclusive kinematics at the hadronic vertex, q = k - k_ISR - k' - k_FSR.
///
/// The four-momentum transfer q is taken from the record topology:
///  - between the hard-scattering leptons on the lepton line, q = p(21) - p(23), or the lepton
///    following the status 21 lepton when there is no status 23 lepton (Pythia8);
///  - otherwise from momentum conservation along the lepton line, q = k - k' - sum of all real
///    photons radiated from the lepton line (DJANGOH, BeAGLE, Sartre, ...).
/// When the record stores the exchanged spacelike boson, its momentum is compared with q, and a
/// warning is issued if they differ (e.g. photons not attached to the lepton line). The boson is
/// not used for q itself: EDM4hep stores it as momentum and mass only, and generators write that
/// mass with limited precision.
class InclusiveKinematicsTruthBorn : public InclusiveKinematicsTruthBornAlgorithm,
                                     public WithPodConfig<NoConfig> {

public:
  InclusiveKinematicsTruthBorn(std::string_view name)
      : InclusiveKinematicsTruthBornAlgorithm{
            name,
            {"MCParticles"},
            {"inclusiveKinematics"},
            "Determine Born-level inclusive kinematics from truth information."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
};

} // namespace eicrecon
