// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <ActsExamples/EventData/Trajectories.hpp>
#include <JANA/JEvent.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/TrackProjector.h"
#include "algorithms/tracking/TrackProjectorConfig.h"
#include "extensions/jana/JChainMultifactoryT.h"

namespace eicrecon {

class TrackProjector_factory :
        public JOmniFactory<TrackProjector_factory, TrackProjectorConfig> {

private:
    using AlgoT = eicrecon::TrackProjector;
    std::unique_ptr<AlgoT> m_algo;

    Input<ActsExamples::Trajectories> m_acts_trajectories_input {this};
    PodioOutput<edm4eic::TrackSegment> m_segments_output {this};

    Service<ACTSGeo_service> m_ACTSGeoSvc {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>();
        m_algo->applyConfig(config());
        m_algo->init(m_ACTSGeoSvc().actsGeoProvider(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_segments_output() = m_algo->execute(m_acts_trajectories_input());
    }
};

} // eicrecon
