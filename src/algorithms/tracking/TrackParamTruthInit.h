// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKPARAMTRUTHINIT_H
#define EICRECON_TRACKPARAMTRUTHINIT_H

#include <algorithms/interfaces/IObjectProducer.h>
#include <algorithms/tracking/JugTrack/Track.hpp>
#include <edm4hep/MCParticle.h>
#include <spdlog/logger.h>
#include "TrackParamTruthInitConfig.h"

#include <random>
#include <TDatabasePDG.h>

namespace eicrecon {
    class TrackParamTruthInit: eicrecon::IObjectProducer<edm4hep::MCParticle, eicrecon::TrackParameters> {

    public:

        void init(const std::shared_ptr<spdlog::logger> &logger);

        eicrecon::TrackParameters * produce(const edm4hep::MCParticle *) override;

    private:
        std::shared_ptr<spdlog::logger> m_log;
        TrackParamTruthInitConfig m_cfg;
        std::shared_ptr<TDatabasePDG> m_pdg_db;

        double m_momentum_split{0.0};
        double m_momentum_smear{0.0};

        std::default_random_engine generator; // TODO: need something more appropriate here
        std::uniform_int_distribution<int> m_uniformIntDist{-1, 1}; // defaults to min=-1, max=1

    };
}   // namespace eicrecon

#endif //EICRECON_TRACKPARAMTRUTHINIT_H
