// Original licence header: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks


#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>

#include "ParticlesWithPIDConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

using ParticlesWithPIDAlgorithm =
    algorithms::Algorithm<
      algorithms::Input<edm4eic::ReconstructedParticleCollection, edm4eic::CherenkovParticleIDCollection>,
      algorithms::Output<edm4eic::ReconstructedParticleCollection, edm4hep::ParticleIDCollection>
    >;

class ParticlesWithPID : public ParticlesWithPIDAlgorithm, public WithPodConfig<ParticlesWithPIDConfig> {
public:

    ParticlesWithPID(std::string_view name) : ParticlesWithPIDAlgorithm{name, {"inputReconstructedParticlesCollection", "inputCherenkovParticleIDCollection"}, {"outputReconstructedParticlesCollection"}, "Matches tracks to Cherenkov PIDs"} {};

    void init() final;
    void process(const Input&, const Output&) const final;

private:

    bool linkCherenkovPID(
            edm4eic::MutableReconstructedParticle& in_part,
            const edm4eic::CherenkovParticleIDCollection& in_pids,
            edm4hep::ParticleIDCollection& out_pids
            ) const;
};

}
