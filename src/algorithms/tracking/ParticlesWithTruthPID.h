// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks, Dmitry Kalinkin


#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>

#include "ParticlesWithTruthPIDConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

using ParticlesWithTruthPIDAlgorithm =
    algorithms::Algorithm<
      algorithms::Input<edm4hep::MCParticleCollection, edm4eic::TrackCollection>,
      algorithms::Output<edm4eic::ReconstructedParticleCollection, edm4eic::MCRecoParticleAssociationCollection>
    >;

class ParticlesWithTruthPID : public ParticlesWithTruthPIDAlgorithm, public WithPodConfig<ParticlesWithTruthPIDConfig> {
public:

    ParticlesWithTruthPID(std::string_view name) : ParticlesWithTruthPIDAlgorithm{name, {"inputMCParticlesCollection", "inputTracksCollection"}, {"outputReconstructedParticlesCollection", "outputAssociationsCollection"}, "Converts track to particles with associations"} {};

    void init() final;
    void process(const Input&, const Output&) const final;

private:

    void tracePhiToleranceOnce(const double sinPhiOver2Tolerance, double phiTolerance) const;
};

}
