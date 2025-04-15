// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks, Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackCollection.h>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TracksToParticlesAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackCollection,
                      std::optional<edm4eic::MCRecoTrackParticleAssociationCollection>>,
    algorithms::Output<edm4eic::ReconstructedParticleCollection,
                       std::optional<edm4eic::MCRecoParticleAssociationCollection>>>;

class TracksToParticles : public TracksToParticlesAlgorithm, public WithPodConfig<NoConfig> {
public:
  TracksToParticles(std::string_view name)
      : TracksToParticlesAlgorithm{
            name,
            {"inputTracksCollection", "inputTrackAssociationsCollection"},
            {"outputReconstructedParticlesCollection", "outputAssociationsCollection"},
            "Converts track to particles with associations"} {};

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
