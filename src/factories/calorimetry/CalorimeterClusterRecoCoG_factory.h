// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <edm4eic/EDM4eicVersion.h>

#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterClusterRecoCoG_factory : public JOmniFactory<CalorimeterClusterRecoCoG_factory, CalorimeterClusterRecoCoGConfig> {

public:
    using AlgoT = eicrecon::CalorimeterClusterRecoCoG;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::ProtoCluster> m_proto_input {this};
#if EDM4EIC_VERSION_MAJOR >= 7
    PodioInput<edm4eic::MCRecoCalorimeterHitAssociation> m_mchitassocs_input {this};
#else
    PodioInput<edm4hep::SimCalorimeterHit> m_mchits_input {this};
#endif


    PodioOutput<edm4eic::Cluster> m_cluster_output {this};
    PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_assoc_output {this};

    ParameterRef<std::string> m_energyWeight {this, "energyWeight", config().energyWeight};
    ParameterRef<double> m_samplingFraction {this, "samplingFraction", config().sampFrac};
    ParameterRef<double> m_logWeightBase {this, "logWeightBase", config().logWeightBase};
    ParameterRef<std::vector<double>> m_logWeightBaseCoeffs {this, "logWeightBaseCoeffs", config().logWeightBaseCoeffs};
    ParameterRef<double> m_logWeightBase_Eref {this, "logWeightBase_Eref", config().logWeightBase_Eref};
    ParameterRef<bool> m_longitudinalShowerInfoAvailable {this, "longitudinalShowerInfoAvailable", config().longitudinalShowerInfoAvailable};
    ParameterRef<bool> m_enableEtaBounds {this, "enableEtaBounds", config().enableEtaBounds};

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
#if EDM4EIC_VERSION_MAJOR >= 7
        m_algo->process({m_proto_input(), m_mchitassocs_input()},
#else
        m_algo->process({m_proto_input(), m_mchits_input()},
#endif
                        {m_cluster_output().get(), m_assoc_output().get()});
    }
};

} // eicrecon
