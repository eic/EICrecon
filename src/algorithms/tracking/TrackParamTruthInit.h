// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <edm4eic/TrackParametersCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <random>

#include "ActsGeometryProvider.h"
#include "TrackParamTruthInitConfig.h"
#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {
class TrackParamTruthInit : public WithPodConfig<TrackParamTruthInitConfig> {

public:
  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            const std::shared_ptr<spdlog::logger> logger);

  std::unique_ptr<edm4eic::TrackParametersCollection>
  produce(const edm4hep::EventHeaderCollection* header,
          const edm4hep::MCParticleCollection* mcparticles);

private:
  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  const algorithms::UniqueIDGenSvc& m_uid      = algorithms::UniqueIDGenSvc::instance();
};
} // namespace eicrecon
