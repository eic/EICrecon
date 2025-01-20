// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dmitry Kalinkin

#pragma once

#include <DDSegmentation/BitFieldCoder.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TensorCollection.h>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using DIRCParticleIDPreMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::RawTrackerHitCollection, edm4eic::ReconstructedParticleCollection,
                      std::optional<edm4eic::MCRecoParticleAssociationCollection>>,
    algorithms::Output<edm4eic::TensorCollection, edm4eic::TensorCollection,
                       std::optional<edm4eic::TensorCollection>>>;

class DIRCParticleIDPreML : public DIRCParticleIDPreMLAlgorithm, public WithPodConfig<NoConfig> {

public:
  DIRCParticleIDPreML(std::string_view name)
      : DIRCParticleIDPreMLAlgorithm{
            name,
            {"inputDIRCHits", "inputParticles", "inputParticleAssocs"},
            {"outputDIRCFeatureTensor", "outputTrackFeatureTensor", "outputPIDTargetTensor"},
            ""} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();

  const dd4hep::DDSegmentation::BitFieldElement *m_field_module, *m_field_x, *m_field_y;
};

} // namespace eicrecon
