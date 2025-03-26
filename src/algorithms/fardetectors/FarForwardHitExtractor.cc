// SPDX-License-Identifier: LGPL-3.0-or-later

#include "FarForwardHitExtractor.h"
#include "algorithms/fardetectors/FarForwardHitExtractorConfig.h"

void eicrecon::FarForwardHitExtractor::init(std::shared_ptr<spdlog::logger> logger) {
  m_log = logger;
}

std::unique_ptr<edm4hep::MCParticleCollection> eicrecon::FarForwardHitExtractor::execute(
  const edm4eic::MCRecoTrackerHitAssociationCollection *assoc,
  const edm4eic::TrackerHitCollection *rchits) {

  auto mc_particles = std::make_unique<edm4hep::MCParticleCollection>();
  for (const auto &hit: *rchits){
    edm4hep::MutableMCParticle mc_particle;
    mc_particle.setMomentum({static_cast<float>(0), static_cast<float>(0), static_cast<float>(0)});


    mc_particles->push_back(mc_particle);
  }

  return mc_particles;

}
