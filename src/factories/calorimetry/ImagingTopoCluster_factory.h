// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/ImagingTopoCluster.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class ImagingTopoCluster_factory : public JOmniFactory<ImagingTopoCluster_factory, ImagingTopoClusterConfig> {

public:
    using AlgoT = eicrecon::ImagingTopoCluster;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::CalorimeterHit> m_hits_input {this};
    PodioOutput<edm4eic::ProtoCluster> m_protos_output {this};

    ParameterRef<std::vector<double>> m_ldxy {this, "localDistXY", config().localDistXY};
    ParameterRef<std::vector<double>> m_ldep {this, "layerDistEtaPhi", config().layerDistEtaPhi};
    ParameterRef<std::vector<double>> m_ldxy_adjacent {this, "layerDistXY", config().layerDistXY};
    ParameterRef<eicrecon::ImagingTopoClusterConfig::ELayerMode> m_laymode {this, "layerMode", config().layerMode};
    ParameterRef<int> m_nlr {this, "neighbourLayersRange", config().neighbourLayersRange};
    ParameterRef<double> m_sd {this, "sectorDist", config().sectorDist};
    ParameterRef<double> m_mched {this, "minClusterHitEdep", config().minClusterHitEdep};
    ParameterRef<double> m_mcced {this, "minClusterCenterEdep", config().minClusterCenterEdep};
    ParameterRef<double> m_mced {this, "minClusterEdep", config().minClusterEdep};
    ParameterRef<std::size_t> m_mcnh {this, "minClusterNhits", config().minClusterNhits};

    Service<AlgorithmsInit_service> m_algorithmsInit {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
        m_algo->applyConfig(config());
        m_algo->init();
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_algo->process({m_hits_input()}, {m_protos_output().get()});
    }
};

} // namespace eicrecon
