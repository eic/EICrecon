// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <edm4eic/TrackerHit.h>
#include "extensions/jana/JOmniFactory.h"
#include "algorithms/tracking/TrackerHitCollector.h"

namespace eicrecon {

/// This factory just collects reconstructed hits edm4eic::TrackerHit from different sources
/// And makes a single array out of them
class TrackerHitCollector_factory : public JOmniFactory<TrackerHitCollector_factory> {
public:
    using AlgoT = TrackerHitCollector;

private:
    std::unique_ptr<AlgoT> m_algo;

    VariadicPodioInput<edm4eic::TrackerHit> m_inputs {this};
    PodioOutput<edm4eic::TrackerHit> m_output {this};

public:

    void Configure() {
        m_algo = std::make_unique<AlgoT>();
        m_algo->init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        // TODO: NWB: optional variadic inputs? m_log->debug(e.what());
        m_output() = m_algo->process(m_inputs());
    }
};

} // eicrecon
