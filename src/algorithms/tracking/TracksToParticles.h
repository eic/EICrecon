// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks, Dmitry Kalinkin


#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "TracksToParticlesConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

using TracksToParticlesAlgorithm =
    algorithms::Algorithm<
      algorithms::Input<edm4hep::MCParticleCollection, edm4eic::TrackCollection>,
      algorithms::Output<edm4eic::ReconstructedParticleCollection, edm4eic::MCRecoParticleAssociationCollection>
    >;

class TracksToParticles : public TracksToParticlesAlgorithm, public WithPodConfig<TracksToParticlesConfig> {
public:

    TracksToParticles(std::string_view name) : TracksToParticlesAlgorithm{name, {"inputMCParticlesCollection", "inputTracksCollection"}, {"outputReconstructedParticlesCollection", "outputAssociationsCollection"}, "Converts track to particles with associations"} {};

    void init() final;
    void process(const Input&, const Output&) const final;

private:

    void tracePhiToleranceOnce(const double sinPhiOver2Tolerance, double phiTolerance) const;
};

}
