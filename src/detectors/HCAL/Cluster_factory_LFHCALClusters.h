// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterClusterRecoCoG.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>


// Dummy factory for JFactoryGeneratorT
class Association_factory_LFHCALClustersAssociations : public JFactoryT<edm4eic::MCRecoClusterParticleAssociation> {

public:
    //------------------------------------------
    // Constructor
    Association_factory_LFHCALClustersAssociations(){
        SetTag("LFHCALClustersAssociations");
    }
};


class Cluster_factory_LFHCALClusters : public JFactoryT<edm4eic::Cluster>, CalorimeterClusterRecoCoG {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_LFHCALClusters(){
        SetTag("LFHCALClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        //-------- Configuration Parameters ------------
        m_input_simhit_tag="LFHCALHits";
        m_input_protoclust_tag="LFHCALIslandProtoClusters";
    
        m_sampFrac=1.;
        m_logWeightBase=4.5;
        m_depthCorrection=0.0;
        m_energyWeight="log";
        m_moduleDimZName="";
        // Constrain the cluster position eta to be within
        // the eta of the contributing hits. This is useful to avoid edge effects
        // for endcaps.
        m_enableEtaBounds=false;

        app->SetDefaultParameter("HCAL:LFHCALClusters:samplingFraction",             m_sampFrac);
        app->SetDefaultParameter("HCAL:LFHCALClusters:logWeightBase",  m_logWeightBase);
        app->SetDefaultParameter("HCAL:LFHCALClusters:depthCorrection",     m_depthCorrection);
        app->SetDefaultParameter("HCAL:LFHCALClusters:input_simhit_tag", m_input_simhit_tag);
        app->SetDefaultParameter("HCAL:LFHCALClusters:input_protoclust_tag", m_input_protoclust_tag);
        app->SetDefaultParameter("HCAL:LFHCALClusters:energyWeight",   m_energyWeight);
        app->SetDefaultParameter("HCAL:LFHCALClusters:moduleDimZName",   m_moduleDimZName);
        app->SetDefaultParameter("HCAL:LFHCALClusters:enableEtaBounds",   m_enableEtaBounds);

        m_geoSvc = app->template GetService<JDD4hep_service>();

        AlgorithmInit(m_log);
    }

    //------------------------------------------
    // ChangeRun
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override{
        AlgorithmChangeRun();
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override{

        
        // Prefill inputs
        m_inputSimhits=event->Get<edm4hep::SimCalorimeterHit>(m_input_simhit_tag);
        m_inputProto=event->Get<edm4eic::ProtoCluster>(m_input_protoclust_tag); 

        // Call Process for generic algorithm
        AlgorithmProcess();


        //outputs

        // Hand owner of algorithm objects over to JANA
        Set(m_outputClusters);
        event->Insert(m_outputAssociations, "LFHCALClusterAssociations");
        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};

