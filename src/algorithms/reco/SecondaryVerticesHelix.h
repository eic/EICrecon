// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Xin Dong

#pragma once

#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/VertexCollection.h>
#include <gsl/pointers>
#include <string>      // for basic_string
#include <string_view> // for string_view

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/reco/SecondaryVerticesHelixConfig.h"

namespace eicrecon {

using SecondaryVerticesHelixAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::VertexCollection, edm4eic::ReconstructedParticleCollection>,
    algorithms::Output<edm4eic::VertexCollection>>;

class SecondaryVerticesHelix : public SecondaryVerticesHelixAlgorithm,
                               public WithPodConfig<SecondaryVerticesHelixConfig> {

public:
  SecondaryVerticesHelix(std::string_view name)
      : SecondaryVerticesHelixAlgorithm{
            name,
            {"inputVertices", "inputParticles"},
            {"outputSecondaryVertices"},
            "Reconstruct secondary vertices in SecondaryVertices collection"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const dd4hep::Detector* m_det{algorithms::GeoSvc::instance().detector()};

  SecondaryVerticesHelixConfig m_cfg;
};

} // namespace eicrecon
