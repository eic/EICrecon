// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <JANA/JMultifactory.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterClusterRecoCoG.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class Cluster_factory_HcalEndcapNClusters : public JMultifactory, CalorimeterClusterRecoCoG {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_HcalEndcapNClusters(){
        DeclarePodioOutput<edm4eic::Cluster>("HcalEndcapNClusters");
        DeclarePodioOutput<edm4eic::MCRecoClusterParticleAssociation>("HcalEndcapNClusterAssociations");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = japp; // GetApplication(); // TODO: NWB: FIXME after JANA2 v2.1.1
        m_log = app->GetService<Log_service>()->logger("HcalEndcapNClusters");

        //-------- Configuration Parameters ------------
        m_input_simhit_tag="HcalEndcapNHits";
        m_input_protoclust_tag="HcalEndcapNIslandProtoClusters";

        m_sampFrac=1.0;// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/bf366a35b480cda6c610b0dd6a4d4efcfd9a8e03/JugReco/src/components/ClusterRecoCoG.cpp
        m_logWeightBase=6.2;// from ATHENA's reconstruction.py
        m_depthCorrection=0.0;// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/bf366a35b480cda6c610b0dd6a4d4efcfd9a8e03/JugReco/src/components/ClusterRecoCoG.cpp
        m_energyWeight="log";// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/bf366a35b480cda6c610b0dd6a4d4efcfd9a8e03/JugReco/src/components/ClusterRecoCoG.cpp
        m_moduleDimZName="";// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/bf366a35b480cda6c610b0dd6a4d4efcfd9a8e03/JugReco/src/components/ClusterRecoCoG.cpp
        // Constrain the cluster position eta to be within
        // the eta of the contributing hits. This is useful to avoid edge effects
        // for endcaps.
        m_enableEtaBounds=false;// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/bf366a35b480cda6c610b0dd6a4d4efcfd9a8e03/JugReco/src/components/ClusterRecoCoG.cpp


        app->SetDefaultParameter("EHCAL:HcalEndcapNClusters:samplingFraction",             m_sampFrac);
        app->SetDefaultParameter("EHCAL:HcalEndcapNClusters:logWeightBase",  m_logWeightBase);
        app->SetDefaultParameter("EHCAL:HcalEndcapNClusters:depthCorrection",     m_depthCorrection);
        app->SetDefaultParameter("EHCAL:HcalEndcapNClusters:input_simhit_tag", m_input_simhit_tag);
        app->SetDefaultParameter("EHCAL:HcalEndcapNClusters:input_protoclust_tag", m_input_protoclust_tag);
        app->SetDefaultParameter("EHCAL:HcalEndcapNClusters:energyWeight",   m_energyWeight);
        app->SetDefaultParameter("EHCAL:HcalEndcapNClusters:moduleDimZName",   m_moduleDimZName);
        app->SetDefaultParameter("EHCAL:HcalEndcapNClusters:enableEtaBounds",   m_enableEtaBounds);

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
        SetData("HcalEndcapNClusters", m_outputClusters);
        SetData("HcalEndcapNClusterAssociations", m_outputAssociations);

        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};
