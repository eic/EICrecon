// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <string>
#include <string_view>

#include "MatchToRICHPIDConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoParticleLinkCollection.h>
#endif

namespace eicrecon {

using MatchToRICHPIDAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection,
                                            edm4eic::MCRecoParticleAssociationCollection,
                                            edm4eic::CherenkovParticleIDCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                             edm4eic::MCRecoParticleLinkCollection,
#endif
                                             edm4eic::MCRecoParticleAssociationCollection,
                                             edm4hep::ParticleIDCollection>>;

class MatchToRICHPID : public MatchToRICHPIDAlgorithm, public WithPodConfig<MatchToRICHPIDConfig> {
public:
  MatchToRICHPID(std::string_view name)
      : MatchToRICHPIDAlgorithm{name,
                                {"inputReconstructedParticlesCollection",
                                 "inputAssociationsCollection",
                                 "inputCherenkovParticleIDCollection"},
                                {"outputReconstructedParticlesCollection",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                 "outputLinks",
#endif
                                 "outputAssociationsCollection"},
                                "Matches tracks to Cherenkov PIDs"} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  bool linkCherenkovPID(edm4eic::MutableReconstructedParticle& in_part,
                        const edm4eic::CherenkovParticleIDCollection& in_pids,
                        edm4hep::ParticleIDCollection& out_pids) const;
};

} // namespace eicrecon
