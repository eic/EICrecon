// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <edm4eic/TrackParametersCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <random>

#include "ActsGeometryProvider.h"
#include "TrackParamTruthInitConfig.h"
#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {
    class TrackParamTruthInit: public WithPodConfig<TrackParamTruthInitConfig> {

    public:

        void init(std::shared_ptr<const ActsGeometryProvider> geo_svc, const std::shared_ptr<spdlog::logger> logger);

        std::unique_ptr<edm4eic::TrackParametersCollection>
            produce(const edm4hep::MCParticleCollection* parts);

    private:
        std::shared_ptr<spdlog::logger> m_log;
        std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

        const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();

        std::default_random_engine generator; // TODO: need something more appropriate here
        std::uniform_int_distribution<int> m_uniformIntDist{-1, 1}; // defaults to min=-1, max=1
        std::normal_distribution<double> m_normDist;

    };
}   // namespace eicrecon
