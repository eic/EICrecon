// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterHitsMerger.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

class CalorimeterHitsMerger_factory : public JOmniFactory<CalorimeterHitsMerger_factory, CalorimeterHitsMergerConfig> {
private:
    CalorimeterHitsMerger m_algo;

    PodioInput<edm4eic::CalorimeterHit> m_hits_input {this};
    PodioOutput<edm4eic::CalorimeterHit> m_hits_output {this};

    ParameterRef<std::string> m_readout {this, "readout", config().readout};
    ParameterRef<std::vector<std::string>> m_fields {this, "fields", config().fields};
    ParameterRef<std::vector<int>> m_refs {this, "refs", config().refs};

    Service<DD4hep_service> m_geoSvc {this};

public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(m_geoSvc().detector(), m_geoSvc().converter(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_hits_output() = m_algo.process(*m_hits_input());
    }
};

} // eicrecon
