// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "algorithms/calorimetry/CalorimeterTruthClustering.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

class CalorimeterTruthClustering_factory : public JOmniFactory<CalorimeterTruthClustering_factory> {
private:
    CalorimeterTruthClustering m_algo;

    PodioInput<edm4eic::CalorimeterHit> m_rc_hits_input {this};
    PodioInput<edm4hep::SimCalorimeterHit> m_mc_hits_input {this};
    PodioOutput<edm4eic::ProtoCluster> m_proto_clusters_output {this};

public:
    void Configure() {
        m_algo.init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_proto_clusters_output() = m_algo.process(*m_rc_hits_input(), *m_mc_hits_input());
    }
};

} // eicrecon
