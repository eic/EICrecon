// SPDX-License-Identifier: LGPL-3.0-or-later

#include "FarForwardHitExtractor.h"
#include "algorithms/fardetectors/FarForwardHitExtractorConfig.h"

void eicrecon::FarForwardHitExtractor::init(std::shared_ptr<spdlog::logger> logger) {
  m_log = logger;
}

std::unique_ptr<edm4eic::ReconstructedParticleCollection> eicrecon::FarForwardHitExtractor::execute(
  const edm4hep::MCParticleCollection *mcparts,
  const edm4eic::TrackerHitCollection *rchits) {

  auto reconstructed_particles = std::make_unique<edm4eic::ReconstructedParticleCollection>();
  for (const auto &hit: *rchits){
    edm4eic::MutableReconstructedParticle reconstructed_particle;
    reconstructed_particle.setType(0);
    reconstructed_particle.setMomentum({static_cast<float>(0), static_cast<float>(0), static_cast<float>(0)});


    reconstructed_particles->push_back(reconstructed_particle);
  }

  return reconstructed_particles;

}
