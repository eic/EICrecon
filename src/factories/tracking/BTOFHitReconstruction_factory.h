// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/tracking/BTOFHitReconstruction.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class BTOFHitReconstruction_factory :
public JOmniFactory<BTOFHitReconstruction_factory, BTOFHitReconstructionConfig> {
 private:
    eicrecon::BTOFHitReconstruction m_algo;

    PodioInput<edm4eic::RawTrackerHit> m_raw_hits_input {this};
    PodioOutput<edm4eic::TrackerHit> m_rec_hits_output {this};

    Service<DD4hep_service> m_geoSvc {this};

public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(m_geoSvc().converter(), m_geoSvc().detector(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_rec_hits_output() = m_algo.process(*m_raw_hits_input());
    }
};

} // eicrecon
