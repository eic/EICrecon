// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/digi/SiliconTrackerDigi.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

class SiliconTrackerDigi_factory : public JOmniFactory<SiliconTrackerDigi_factory, SiliconTrackerDigiConfig> {

public:
    using AlgoT = eicrecon::SiliconTrackerDigi;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4hep::SimTrackerHit> m_sim_hits_input {this};
    PodioOutput<edm4eic::RawTrackerHit> m_raw_hits_output {this};

    ParameterRef<double> m_threshold {this, "threshold", config().threshold};
    ParameterRef<double> m_timeResolution {this, "timeResolution", config().timeResolution};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->applyConfig(config());
        m_algo->init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_algo->process({m_sim_hits_input()}, {m_raw_hits_output().get()});
    }
};

} // eicrecon
