// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg, Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>

#include "algorithms/reco/ScatteredElectronsEMinusPzConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

  using ScatteredElectronsEMinusPzAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::ReconstructedParticleCollection,
      edm4eic::ReconstructedParticleCollection
    >,
    algorithms::Output<
      edm4eic::ReconstructedParticleCollection
    >
  >;

  class ScatteredElectronsEMinusPz : public ScatteredElectronsEMinusPzAlgorithm, WithPodConfig<ScatteredElectronsEMinusPzConfig>{

  public:

    ScatteredElectronsEMinusPz(std::string_view name)
      : ScatteredElectronsEMinusPzAlgorithm{name,
                            {"inputParticles", "inputElectronCandidates"},
                            {"outputElectrons"},
                            "Outputs DIS electrons ordered in decreasing E-pz"} {}
    void init() final;
    void process(const Input&, const Output&) const final;

  private:
    double m_electron{0.000510998928}, m_pion{0.13957};

  };

} // namespace eicrecon
