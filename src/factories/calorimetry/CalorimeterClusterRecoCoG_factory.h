// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <algorithms/property.h>
#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

using ConfigMap = std::map<std::string_view, algorithms::PropertyValue>;

class CalorimeterClusterRecoCoG_factory : public JOmniFactory<CalorimeterClusterRecoCoG_factory, ConfigMap> {

public:
    using AlgoT = eicrecon::CalorimeterClusterRecoCoG;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::ProtoCluster> m_proto_input {this};
    PodioInput<edm4hep::SimCalorimeterHit> m_mchits_input {this};

    PodioOutput<edm4eic::Cluster> m_cluster_output {this};
    PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_assoc_output {this};

    std::vector<std::unique_ptr<ParameterBase>> m_parameters;

public:
    void Configure() {
      m_algo = std::make_unique<AlgoT>(GetPrefix());

      RegisterAllParameters(m_algo, m_parameters);

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
