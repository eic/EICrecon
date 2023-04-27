// Copyright 2023, Friederike Bock
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <JANA/JMultifactory.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterClusterRecoCoG.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class Cluster_factory_LFHCALClusters : public JMultifactory, CalorimeterClusterRecoCoG {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_LFHCALClusters(){
        DeclarePodioOutput<edm4eic::Cluster>("LFHCALClusters");
        DeclarePodioOutput<edm4eic::MCRecoClusterParticleAssociation>("LFHCALClusterAssociations");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = japp; // GetApplication(); // TODO: NWB: FIXME after JANA2 v2.1.1
        m_log = app->GetService<Log_service>()->logger("LFHCALClusters");

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

        app->SetDefaultParameter("FHCAL:LFHCALClusters:samplingFraction",             m_sampFrac);
        app->SetDefaultParameter("FHCAL:LFHCALClusters:logWeightBase",  m_logWeightBase);
        app->SetDefaultParameter("FHCAL:LFHCALClusters:depthCorrection",     m_depthCorrection);
        app->SetDefaultParameter("FHCAL:LFHCALClusters:input_simhit_tag", m_input_simhit_tag);
        app->SetDefaultParameter("FHCAL:LFHCALClusters:input_protoclust_tag", m_input_protoclust_tag);
        app->SetDefaultParameter("FHCAL:LFHCALClusters:energyWeight",   m_energyWeight);
        app->SetDefaultParameter("FHCAL:LFHCALClusters:moduleDimZName",   m_moduleDimZName);
        app->SetDefaultParameter("FHCAL:LFHCALClusters:enableEtaBounds",   m_enableEtaBounds);

        m_geoSvc = app->template GetService<JDD4hep_service>();

        AlgorithmInit(m_log);
    }

    //------------------------------------------
    // ChangeRun
    void BeginRun(const std::shared_ptr<const JEvent> &event) override{
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

        // Hand owner of algorithm objects over to JANA
        SetData("LFHCALClusters", m_outputClusters);
        SetData("LFHCALClusterAssociations", m_outputAssociations);

        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};
