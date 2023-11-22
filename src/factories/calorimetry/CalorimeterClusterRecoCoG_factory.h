// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <algorithms/calorimetry/CalorimeterClusterRecoCoG.h>
#include <services/geometry/dd4hep/DD4hep_service.h>
#include <extensions/jana/JOmniFactory.h>


namespace eicrecon {

class CalorimeterClusterRecoCoG_factory : public JOmniFactory<CalorimeterClusterRecoCoG_factory, CalorimeterClusterRecoCoGConfig> {

    using AlgoT = eicrecon::CalorimeterClusterRecoCoG;
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::ProtoCluster> m_proto_input {this};
    PodioInput<edm4hep::SimCalorimeterHit> m_mchits_input {this};

    PodioOutput<edm4eic::Cluster> m_cluster_output {this};
    PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_assoc_output {this};

    ParameterRef<std::string> m_energyWeight {this, "energyWeight", config().energyWeight};
    ParameterRef<double> m_samplingFraction {this, "samplingFraction", config().sampFrac};
    ParameterRef<double> m_logWeightBase {this, "logWeightBase", config().logWeightBase};
    ParameterRef<bool> m_enableEtaBounds {this, "enableEtaBounds", config().enableEtaBounds};

    Service<DD4hep_service> m_geoSvc {this};
    // Resource<DD4hep_service, const dd4hep::Detector*> m_detector {this, [](std::shared_ptr<DD4hep_service> s, int64_t run_nr){ return s->detector(); }}

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->applyConfig(config());
        m_algo->init(m_geoSvc().detector(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_cluster_output() = std::move(std::make_unique<PodioTypeMap<edm4eic::Cluster>::collection_t>());
        m_assoc_output() = std::move(std::make_unique<PodioTypeMap<edm4eic::MCRecoClusterParticleAssociation>::collection_t>());

        m_algo->process({m_proto_input(), m_mchits_input()},
                        {m_cluster_output().get(), m_assoc_output().get()});
    }
};

} // eicrecon
