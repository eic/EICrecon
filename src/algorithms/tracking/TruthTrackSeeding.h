// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRUTHTRACKSEEDING_H
#define EICRECON_TRUTHTRACKSEEDING_H

#include <algorithms/interfaces/IObjectProducer.h>
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
        TDatabasePDG * m_pdg_db;
    };
}


#endif //EICRECON_TRUTHTRACKSEEDING_H
