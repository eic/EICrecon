// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg, Xin Dong

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <string>      // for basic_string
#include <string_view> // for string_view

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/reco/SecondaryVerticesConfig.h"

namespace eicrecon {

using SecondaryVerticesAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::VertexCollection, edm4eic::ReconstructedParticleCollection>,
    algorithms::Output<edm4eic::VertexCollection>>;

class SecondaryVertices : public SecondaryVerticesAlgorithm,
                        public WithPodConfig<SecondaryVerticesConfig> {

public:
  SecondaryVertices(std::string_view name)
      : SecondaryVerticesAlgorithm{name,
                                 {"inputVertices", "inputParticles"},
                                 {"outputSecondaryVertices"},
                                 "Reconstruct secondary vertices in SecondaryVertices collection"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  SecondaryVerticesConfig m_cfg;
};
} // namespace eicrecon
