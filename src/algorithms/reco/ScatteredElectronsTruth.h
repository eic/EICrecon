// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>


namespace eicrecon {

  using ScatteredElectronsTruthAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4hep::MCParticleCollection,
      edm4eic::ReconstructedParticleCollection,
      edm4eic::MCRecoParticleAssociationCollection
    >,
    algorithms::Output<
      edm4eic::ReconstructedParticleCollection
    >
  >;

  class ScatteredElectronsTruth
  : public ScatteredElectronsTruthAlgorithm {

  public:
    ScatteredElectronsTruth(std::string_view name)
      : ScatteredElectronsTruthAlgorithm{name,
                            {"MCParticles", "inputParticles", "inputAssociations"},
                            {"ReconstructedParticles"},
                            "Output a list of possible scattered electrons using truth MC Particle associations."} {}

    void init() final;
    void process(const Input&, const Output&) const final;

  private:
    double m_proton{0.93827}, m_neutron{0.93957}, m_electron{0.000510998928}, m_pion{0.13957};
  };

} // namespace eicrecon
