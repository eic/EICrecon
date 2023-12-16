// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

#pragma once

#include <algorithms/calorimetry/LogWeightReco.h>
#include <services/geometry/dd4hep/DD4hep_service.h>
#include <extensions/jana/JOmniFactory.h>


namespace eicrecon {

class LogWeightReco_factory : public JOmniFactory<LogWeightReco_factory, LogWeightRecoConfig> {
private:
    LogWeightReco m_algo;
    PodioInput<edm4eic::ProtoCluster> m_proto_cluster_input {this};             
    PodioOutput<edm4eic::Cluster> m_cluster_output {this};

    ParameterRef<double> m_sf	{this, "sampling_fraction", config().sampling_fraction};
    ParameterRef<double> m_E0	{this, "E0",                config().E0};
    ParameterRef<double> m_w0_a {this, "w0_a",              config().w0_a};
    ParameterRef<double> m_w0_b {this, "w0_b",              config().w0_b};
    ParameterRef<double> m_w0_c {this, "w0_c",              config().w0_c};

    Service<DD4hep_service> m_geoSvc {this};

public:
    void Configure() {                                                          
        m_algo.applyConfig(config());                                           
        m_algo.init(m_geoSvc().detector(), logger());   
    }                                                                           
                                                                                
    void ChangeRun(int64_t run_number) {                                        
    }                                                                           
                                                                                
    void Process(int64_t run_number, uint64_t event_number) {                   
        m_cluster_output() = m_algo.process(*m_proto_cluster_input());              
    }            

};

} // eicrecon
