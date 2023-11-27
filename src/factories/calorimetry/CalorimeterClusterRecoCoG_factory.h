// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <algorithms/property.h>
#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

using ConfigMap = std::map<std::string, algorithms::PropertyValue>;

class CalorimeterClusterRecoCoG_factory : public JOmniFactory<CalorimeterClusterRecoCoG_factory, ConfigMap> {

public:
    using AlgoT = eicrecon::CalorimeterClusterRecoCoG;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::ProtoCluster> m_proto_input {this};
    PodioInput<edm4hep::SimCalorimeterHit> m_mchits_input {this};

    PodioOutput<edm4eic::Cluster> m_cluster_output {this};
    PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_assoc_output {this};
/*
    std::vector<std::unique_ptr<ParameterBase>> m_parameters = [this](){
      std::vector<std::unique_ptr<ParameterBase>> v;
      v.push_back(std::make_unique<ParameterRef<std::string>>(this, "energyWeight", std::string("log")));
      return v;
    }();
*/
    ParameterRef<std::string> m_energyWeight {this, "energyWeight", std::get<std::string>(config()["energyWeight"])};
    ParameterRef<double> m_samplingFraction {this, "samplingFraction", std::get<double>(config()["samplingFraction"])};
    ParameterRef<double> m_logWeightBase {this, "logWeightBase", std::get<double>(config()["logWeightBase"])};
    ParameterRef<bool> m_enableEtaBounds {this, "enableEtaBounds", std::get<bool>(config()["enableEtaBounds"])};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->init(logger());
        m_algo->init();
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_algo->process({m_proto_input(), m_mchits_input()},
                        {m_cluster_output().get(), m_assoc_output().get()});
    }
};

} // eicrecon
