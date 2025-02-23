#pragma once

#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <spdlog/logger.h>

#include "FarForwardHitExtractorConfig.h"
#include "algorithms/algorithm.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  class FarForwardHitExtractor :  public WithPodConfig<FarForwardHitExtractorConfig> {

  public:

    // Initialization will set the pointer of the logger
    void init(std::shared_ptr<spdlog::logger> logger);
    std::unique_ptr<edm4hep::MCParticleCollection> execute(
      const edm4eic::MCRecoTrackerHitAssociationCollection *assoc,
      const edm4eic::TrackerHitCollection *rchits);

  private:
    // pointer to logger
    std::shared_ptr<spdlog::logger>   m_log;
  };
}
