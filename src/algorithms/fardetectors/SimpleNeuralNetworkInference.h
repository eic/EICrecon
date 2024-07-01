#pragma once

#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <spdlog/logger.h>

#include "SimpleNeuralNetworkInferenceConfig.h"
#include "algorithms/algorithm.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  class SimpleNeuralNetworkInference :  public WithPodConfig<SimpleNeuralNetworkInferenceConfig> {

  public:

    // Initialization will set the pointer of the logger
    void init(std::shared_ptr<spdlog::logger> logger);
    std::unique_ptr<edm4eic::ReconstructedParticleCollection> execute(const edm4eic::TrackerHitCollection *rchits) const;

  private:
    // pointer to logger
    std::shared_ptr<spdlog::logger>   m_log; 
  };
}
