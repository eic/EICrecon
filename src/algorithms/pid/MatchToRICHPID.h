// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks

#pragma once

#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

#include "MatchToRICHPIDConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace edm4eic {
class CherenkovParticleIDCollection;
}
namespace edm4eic {
class MCRecoParticleAssociationCollection;
}
namespace edm4eic {
class MutableReconstructedParticle;
}
namespace edm4eic {
class ReconstructedParticleCollection;
}
namespace edm4hep {
class ParticleIDCollection;
}

namespace eicrecon {

using MatchToRICHPIDAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection,
                                            edm4eic::MCRecoParticleAssociationCollection,
                                            edm4eic::CherenkovParticleIDCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection,
                                             edm4eic::MCRecoParticleAssociationCollection,
                                             edm4hep::ParticleIDCollection>>;

class MatchToRICHPID : public MatchToRICHPIDAlgorithm, public WithPodConfig<MatchToRICHPIDConfig> {
public:
  MatchToRICHPID(std::string_view name)
      : MatchToRICHPIDAlgorithm{
            name,
            {"inputReconstructedParticlesCollection", "inputAssociationsCollection",
             "inputCherenkovParticleIDCollection"},
            {"outputReconstructedParticlesCollection", "outputAssociationsCollection"},
            "Matches tracks to Cherenkov PIDs"} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  bool linkCherenkovPID(edm4eic::MutableReconstructedParticle& in_part,
                        const edm4eic::CherenkovParticleIDCollection& in_pids,
                        edm4hep::ParticleIDCollection& out_pids) const;
};

} // namespace eicrecon
