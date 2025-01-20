// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/TensorCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using DIRCParticleIDPostMLAlgorithm =
  algorithms::Algorithm<
    algorithms::Input<edm4eic::ReconstructedParticleCollection,
                                          std::optional<edm4eic::MCRecoParticleAssociationCollection>,
                                          edm4eic::TensorCollection>,
    algorithms::Output<edm4eic::ReconstructedParticleCollection,
                      std::optional<edm4eic::MCRecoParticleAssociationCollection>,
                                          edm4hep::ParticleIDCollection>
  >;

class DIRCParticleIDPostML : public DIRCParticleIDPostMLAlgorithm,
                             public WithPodConfig<NoConfig> {

public:
  DIRCParticleIDPostML(std::string_view name)
      : DIRCParticleIDPostMLAlgorithm{name,
                                                                                         {"inputParticles", "inputParticleAssociations", "inputPredictionsTensor"},
                                                                                         {"outputParticles", "outputParticleAssociations", "outputParticleIDs"},
                                                                                         ""} {
  }

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
