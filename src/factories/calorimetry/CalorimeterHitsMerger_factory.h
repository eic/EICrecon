// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterHitsMerger.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

class CalorimeterHitsMerger_factory : public JOmniFactory<CalorimeterHitsMerger_factory, CalorimeterHitsMergerConfig> {
private:
    CalorimeterHitsMerger m_algo;

    PodioInput<edm4eic::CalorimeterHit> m_hits_input {this};
    PodioOutput<edm4eic::CalorimeterHit> m_hits_output {this};

    ParameterRef {this, "fields", config().fields};
    app->SetDefaultParameter(param_prefix + "refs", config().refs);

public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(m_geoSvc().detector(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_hits_output() = m_algo.process(m_hits_input());
    }
};

} // eicrecon
