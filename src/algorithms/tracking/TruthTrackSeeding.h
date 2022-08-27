// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRUTHTRACKSEEDING_H
#define EICRECON_TRUTHTRACKSEEDING_H

#include <algorithms/interfaces/IObjectProducer.h>
#include <edm4hep/MCParticle.h>
#include <eicd/TrackParameters.h>
#include <TDatabasePDG.h>


namespace eicrecon {
    class TruthTrackSeeding:
            public eicrecon::IObjectProducer<edm4hep::MCParticle, eicd::TrackParameters> {
    public:
        void init();

        eicd::TrackParameters * produce(const edm4hep::MCParticle *) override;

    private:
        std::shared_ptr<TDatabasePDG> m_pdg_db;
    };
}


#endif //EICRECON_TRUTHTRACKSEEDING_H
