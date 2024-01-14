// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include "algorithms/digi/SplitGeometry.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"


namespace eicrecon {

class SplitGeometry_factory : public JOmniFactory<SplitGeometry_factory, SplitGeometryConfig> {

    SplitGeometry m_algo;

    PodioInput<edm4eic::RawTrackerHit> m_raw_hits_input {this};
    VariadicPodioOutput<edm4eic::RawTrackerHit> m_split_hits_output {this};

    Service<DD4hep_service> m_geoSvc {this};

public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(m_geoSvc().detector(),logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_split_hits_output() = m_algo.process(*m_raw_hits_input());
    }
};

} // eicrecon
