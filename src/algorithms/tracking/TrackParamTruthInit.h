// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <algorithms/interfaces/IObjectProducer.h>
#include <algorithms/tracking/JugTrack/Track.hpp>
#include <edm4hep/MCParticle.h>
#include <spdlog/logger.h>

#include "TrackParamTruthInitConfig.h"
#include <algorithms/interfaces/WithPodConfig.h>

#include <random>
#include <TDatabasePDG.h>

namespace eicrecon {
    class TrackParamTruthInit: public WithPodConfig<TrackParamTruthInitConfig> {

    public:

        void init(const std::shared_ptr<spdlog::logger> &logger);

        eicrecon::TrackParameters * produce(const edm4hep::MCParticle *);

    private:
        std::shared_ptr<spdlog::logger> m_log;
        std::shared_ptr<TDatabasePDG> m_pdg_db;

        std::default_random_engine generator; // TODO: need something more appropriate here
        std::uniform_int_distribution<int> m_uniformIntDist{-1, 1}; // defaults to min=-1, max=1
        std::normal_distribution<double> m_normDist;

    };
}   // namespace eicrecon
