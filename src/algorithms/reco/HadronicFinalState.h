// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tyler Kutz

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

using HadronicFinalStateAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4hep::MCParticleCollection,
                      edm4hep::MCParticleCollection, edm4eic::ReconstructedParticleCollection,
                      edm4eic::MCRecoParticleAssociationCollection>,
    algorithms::Output<edm4eic::HadronicFinalStateCollection>>;

class HadronicFinalState : public HadronicFinalStateAlgorithm, public WithPodConfig<NoConfig> {

public:
  HadronicFinalState(std::string_view name)
      : HadronicFinalStateAlgorithm{name,
                                    {"MCBeamElectrons", "MCBeamProtons", "MCParticles",
                                     "inputParticles", "inputAssociations"},
                                    {"hadronicFinalState"},
                                    "Calculate summed quantities of the hadronic final state."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  double m_crossingAngle{-0.025};
};

} // namespace eicrecon
