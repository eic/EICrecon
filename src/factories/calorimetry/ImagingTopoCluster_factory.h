// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/ImagingTopoCluster.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

class ImagingTopoCluster_factory : public JOmniFactory<ImagingTopoCluster_factory, ImagingTopoClusterConfig> {
private:
    eicrecon::ImagingTopoCluster m_algo;

    PodioInput<edm4eic::CalorimeterHit> m_hits_input {this};
    PodioOutput<edm4eic::ProtoCluster> m_protos_output {this};

    ParameterRef m_ldxy {this, "localDistXY", config().localDistXY};
    ParameterRef m_ldep {this, "layerDistEtaPhi", config().layerDistEtaPhi};
    ParameterRef m_nlr {this, "neighbourLayersRange", config().neighbourLayersRange};
    ParameterRef m_sd {this, "sectorDist", config().sectorDist};
    ParameterRef m_mched {this, "minClusterHitEdep", config().minClusterHitEdep};
    ParameterRef m_mcced {this, "minClusterCenterEdep", config().minClusterCenterEdep};
    ParameterRef m_mced {this, "minClusterEdep", config().minClusterEdep};
    ParameterRef m_mcnh {this, "minClusterNhits", config().minClusterNhits};

public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_protos_output() = m_algo.process(m_hits_input());
    }
};

} // namespace eicrecon
