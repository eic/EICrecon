#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <spdlog/logger.h>

#include "SimpleNeuralNetworkInference.h"
#include "algorithms/algorithm.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  class TorchScriptInterface :  public WithPodConfig<SimpleNeuralNetworkInference> {

  public:
    void init(std::shared_ptr<spdlog::logger> logger);
    std::unique_ptr<edm4eic::ReconstructedParticleCollection> execute(const edm4eic::TrackerHitCollection *rchits) const;

  private:

    std::shared_ptr<spdlog::logger>   m_log;

  };
}
