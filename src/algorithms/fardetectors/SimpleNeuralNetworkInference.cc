// SPDX-License-Identifier: LGPL-3.0-or-later

#include "SimpleNeuralNetworkInference.h"
#include "algorithms/fardetectors/SimpleNeuralNetworkInferenceConfig.h"

void eicrecon::SimpleNeuralNetworkInference::init(std::shared_ptr<spdlog::logger> logger) {
  m_log = logger;
}

std::unique_ptr<edm4eic::ReconstructedParticleCollection> eicrecon::SimpleNeuralNetworkInference::execute(const edm4eic::TrackerHitCollection *rchits) const{

  auto reconstructed_particles = std::make_unique<edm4eic::ReconstructedParticleCollection>();
  return reconstructed_particles;

}
