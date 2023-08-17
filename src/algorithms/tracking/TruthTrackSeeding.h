// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "algorithms/interfaces/IObjectProducer.h"
#include <edm4hep/MCParticle.h>
#include <edm4eic/TrackParameters.h>
#include <TDatabasePDG.h>


namespace eicrecon {
    class TruthTrackSeeding:
            public eicrecon::IObjectProducer<edm4hep::MCParticle, edm4eic::TrackParameters> {
    public:
        void init();

        edm4eic::TrackParameters * produce(const edm4hep::MCParticle *) override;

    private:
        std::shared_ptr<TDatabasePDG> m_pdg_db;
    };
}
