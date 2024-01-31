// Created by Shujie Li
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/Measurement2DCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/TrackerMeasurementFromHits.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class TrackerMeasurementFromHits_factory :
        public JOmniFactory<TrackerMeasurementFromHits_factory> {

private:
    using AlgoT = eicrecon::TrackerMeasurementFromHits;
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::TrackerHit> m_hits_input {this};
    PodioOutput<edm4eic::Measurement2D> m_measurements_output {this};

    Service<DD4hep_service> m_DD4hepSvc {this};
    Service<ACTSGeo_service> m_ACTSGeoSvc {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>();
        m_algo->init(m_DD4hepSvc().detector(), m_DD4hepSvc().converter(), m_ACTSGeoSvc().actsGeoProvider(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_measurements_output() = m_algo->produce(*m_hits_input());
    }
};

} // eicrecon
