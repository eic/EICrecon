// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "InclusiveKinematicsTruthDressedConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

using InclusiveKinematicsTruthDressedAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection>,
    algorithms::Output<edm4eic::InclusiveKinematicsCollection,
                       edm4eic::ReconstructedParticleCollection, edm4hep::MCParticleCollection>>;

/// Inclusive kinematics on the lepton side with a dressed scattered lepton,
/// q = k - (k' + k_FSR), where k_FSR are the real photons radiated from the scattered lepton.
///
/// Outputs the kinematics, the dressed scattered lepton (as the `scat` relation of the
/// kinematics), and the generator particles it is made of (bare lepton first, then photons).
class InclusiveKinematicsTruthDressed
    : public InclusiveKinematicsTruthDressedAlgorithm,
      public WithPodConfig<InclusiveKinematicsTruthDressedConfig> {

public:
  InclusiveKinematicsTruthDressed(std::string_view name)
      : InclusiveKinematicsTruthDressedAlgorithm{
            name,
            {"MCParticles"},
            {"inclusiveKinematics", "scatteredLepton", "scatteredLeptonConstituents"},
            "Determine inclusive kinematics with a dressed scattered lepton from truth "
            "information."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  enum class PhotonAssignment { Auto, Parentage, Cone };
  PhotonAssignment m_photonAssignment{PhotonAssignment::Auto};

  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
};

} // namespace eicrecon
