// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/digi/SplitGeometry.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

class SplitGeometry_factory : public JOmniFactory<SplitGeometry_factory, SplitGeometryConfig> {

    SplitGeometry m_algo;

    PodioInput<edm4hep::RawTrackerHit> m_raw_hits_input {this};
    VariadicPodioOutput<edm4eic::RawTrackerHit> m_split_hits_output {this};

public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_split_hits_output() = m_algo.process(*m_raw_hits_input());
    }
};

} // eicrecon
