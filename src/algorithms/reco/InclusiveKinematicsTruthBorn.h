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
/// The four-momentum transfer q is taken, in order of preference, from:
///  - the exchanged spacelike boson emitted from the lepton line (DJANGOH, BeAGLE), with its
///    energy from its stored momentum and mass; when the balance along the lepton line,
///    k - k' - sum of photons, agrees with the boson within the precision of the stored mass,
///    that balance is used, since it determines a small boson energy more precisely;
///  - the hard-scattering leptons on the lepton line, q = p(21) - p(23), or the lepton following
///    the status 21 lepton when there is no status 23 lepton (Pythia8);
///  - the beam and scattered leptons and all real photons radiated from the lepton line.
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
