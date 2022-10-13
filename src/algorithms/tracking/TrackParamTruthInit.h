// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKPARAMTRUTHINIT_H
#define EICRECON_TRACKPARAMTRUTHINIT_H

#include <algorithms/interfaces/IObjectProducer.h>
#include <algorithms/interfaces/WithPodConfig.h>
#include <algorithms/tracking/JugTrack/Track.hpp>
#include <edm4hep/MCParticle.h>
#include <spdlog/logger.h>
#include "TrackParamTruthInitConfig.h"

#include <TDatabasePDG.h>

namespace eicrecon {
    class TrackParamTruthInit:
            public eicrecon::IObjectProducer<edm4hep::MCParticle, Jug::TrackParameters>,
            public WithPodConfig<TrackParamTruthInitConfig> {

    public:

        void init(const std::shared_ptr<spdlog::logger> &logger);

        Jug::TrackParameters * produce(const edm4hep::MCParticle *) override;

    private:
        std::shared_ptr<spdlog::logger> m_log;
        std::shared_ptr<TDatabasePDG> m_pdg_db;
    };
}   // namespace eicrecon

#endif //EICRECON_TRACKPARAMTRUTHINIT_H
