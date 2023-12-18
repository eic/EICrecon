// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

#pragma once

#include <algorithms/calorimetry/HEXPLIT.h>
#include <services/geometry/dd4hep/DD4hep_service.h>
#include <extensions/jana/JOmniFactory.h>
// #include <extensions/spdlog/SpdlogMixin.h>


namespace eicrecon {

class HEXPLIT_factory : public JOmniFactory<HEXPLIT_factory, HEXPLITConfig> {
private:
    HEXPLIT m_algo;
    PodioInput<edm4eic::CalorimeterHit> m_rec_hits_input {this};             
    PodioOutput<edm4eic::CalorimeterHit> m_subcell_hits_output {this};

    ParameterRef<double> m_MIP	    {this, "MIP",           config().MIP};
    ParameterRef<double> m_Emin_in_MIPs     {this, "Emin_in_MIPs",          config().Emin_in_MIPs};
    ParameterRef<double> m_tmax     {this, "tmax",          config().tmax};
    ParameterRef<double> m_sl	    {this, "side_length",   config().side_length};
    ParameterRef<double> m_ls	    {this, "layer_spacing", config().layer_spacing};
    ParameterRef<double> m_rot_x    {this, "rot_x",         config().rot_x};
    ParameterRef<double> m_rot_y    {this, "rot_y",         config().rot_y};
    ParameterRef<double> m_rot_z    {this, "rot_z",         config().rot_z};
    ParameterRef<double> m_trans_x  {this, "trans_x",       config().trans_x};
    ParameterRef<double> m_trans_y  {this, "trans_y",       config().trans_y};
    ParameterRef<double> m_trans_z  {this, "trans_z",       config().trans_z};

    Service<DD4hep_service> m_geoSvc {this};

public:
    void Configure() {                                                          
        m_algo.applyConfig(config());                                           
        m_algo.init(m_geoSvc().detector(), logger());   
    }                                                                           
                                                                                
    void ChangeRun(int64_t run_number) {                                        
    }                                                                           
                                                                                
    void Process(int64_t run_number, uint64_t event_number) {                   
        m_subcell_hits_output() = m_algo.process(*m_rec_hits_input());              
    }            
};

} // eicrecon
