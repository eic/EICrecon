// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <algorithms/calorimetry/CalorimeterClusterRecoCoG.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <extensions/jana/JOmniFactory.h>


namespace eicrecon {

class CalorimeterClusterRecoCoG_factory : public JOmniFactory<CalorimeterClusterRecoCoG_factory, CalorimeterClusterRecoCoGConfig> {

    eicrecon::CalorimeterClusterRecoCoG m_algo;

    PodioInput<edm4eic::ProtoCluster> m_proto_input {this};
    PodioInput<edm4hep::SimCalorimeterHit> m_mchits_input {this};

    PodioOutput<edm4eic::Cluster> m_cluster_output {this};
    PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_assoc_output {this};

    ParameterRef<double> m_samplingFraction {this, "samplingFraction", GetConfig().sampFrac};
    ParameterRef<double> m_logWeightBase {this, "logWeightBase", GetConfig().logWeightBase};
    ParameterRef<double> m_depthCorrection {this, "depthCorrection", GetConfig().depthCorrection};
    ParameterRef<std::string> m_energyWeight {this, "energyWeight", GetConfig().energyWeight};
    ParameterRef<std::string> m_moduleDimZName {this, "moduleDimZName", GetConfig().moduleDimZName};
    ParameterRef<bool> m_enableEtaBounds {this, "enableEtaBounds", GetConfig().enableEtaBounds};

    Service<JDD4hep_service> m_geoSvc {this};
    // Resource<JDD4hep_service, const dd4hep::Detector*> m_detector {this, [](std::shared_ptr<JDD4hep_service> s, int64_t run_nr){ return s->detector(); }}

public:
    void Configure() {
        m_algo.applyConfig(GetConfig());
        m_algo.init(m_geoSvc().detector(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        std::tie(m_cluster_output(), m_assoc_output()) = m_algo.process(m_proto_input(), m_mchits_input());
    }
};

} // eicrecon
