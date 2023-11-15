// Created by Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <DDRec/CellIDPositionConverter.h>
#include <services/geometry/dd4hep/DD4hep_service.h>
#include <algorithms/fardetectors/MatrixTransferStatic.h>
#include <algorithms/fardetectors/MatrixTransferStaticConfig.h>

// Event Model related classes
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>

#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class MatrixTransferStatic_factory :
    public JOmniFactory<MatrixTransferStatic_factory, MatrixTransferStaticConfig> {

    eicrecon::MatrixTransferStatic   m_reco_algo;        // Actual algorithm

    PodioInput<edm4hep::SimTrackerHit> m_hits_input {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_tracks_output {this};

    Service<DD4hep_service> m_geoSvc {this};

    // TODO: Register the config fields with JANA so we can pass them in as parameters

public:
    void Configure() {
        m_reco_algo.applyConfig(config());
        m_reco_algo.init(m_geoSvc()->detector(), m_geoSvc()->converter(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_tracks_output() = m_algo.process(m_hits_input());
    }

};

} // eicrecon
