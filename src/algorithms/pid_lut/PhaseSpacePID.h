// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, Simon Gardner

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/Vector3f.h>
#include <stdint.h>
#include <string>
#include <string_view>

#include "PhaseSpacePIDConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using PhaseSpacePIDAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection,
                                            edm4eic::MCRecoParticleAssociationCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection,
                                             edm4eic::MCRecoParticleAssociationCollection,
                                             edm4hep::ParticleIDCollection>>;

class PhaseSpacePID : public PhaseSpacePIDAlgorithm, public WithPodConfig<PhaseSpacePIDConfig> {

public:
  PhaseSpacePID(std::string_view name)
      : PhaseSpacePIDAlgorithm{name,
                               {"inputParticlesCollection", "inputParticleAssociationsCollection"},
                               {"outputParticlesCollection", "outputParticleAssociationsCollection",
                                "outputParticleIDCollection"},
                               ""} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  int32_t m_system;
  edm4hep::Vector3f m_direction; // Direction vector for the phase space
  double m_mass;
  double m_charge;
};

} // namespace eicrecon
