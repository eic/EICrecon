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
class Association_factory_HcalEndcapPClusterAssociations : public JFactoryT<edm4eic::MCRecoClusterParticleAssociation> {

public:
    //------------------------------------------
    // Constructor
    Association_factory_HcalEndcapPClusterAssociations(){
        SetTag("HcalEndcapPClusterAssociations");
    }
};


class Cluster_factory_HcalEndcapPClusters : public JFactoryT<edm4eic::Cluster>, CalorimeterClusterRecoCoG {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_HcalEndcapPClusters(){
        SetTag("HcalEndcapPClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        //-------- Configuration Parameters ------------
        m_input_simhit_tag="HcalEndcapPHits";
        m_input_protoclust_tag="HcalEndcapPIslandProtoClusters";

        m_sampFrac=0.025;// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/bf366a35b480cda6c610b0dd6a4d4efcfd9a8e03/JugReco/src/components/ClusterRecoCoG.cpp
        m_logWeightBase=6.2;// from ATHENA's reconstruction.py
        m_depthCorrection=0.0;// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/bf366a35b480cda6c610b0dd6a4d4efcfd9a8e03/JugReco/src/components/ClusterRecoCoG.cpp
        m_energyWeight="log";// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/bf366a35b480cda6c610b0dd6a4d4efcfd9a8e03/JugReco/src/components/ClusterRecoCoG.cpp
        m_moduleDimZName="";// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/bf366a35b480cda6c610b0dd6a4d4efcfd9a8e03/JugReco/src/components/ClusterRecoCoG.cpp
        // Constrain the cluster position eta to be within
        // the eta of the contributing hits. This is useful to avoid edge effects
        // for endcaps.
        m_enableEtaBounds=false;// https://eicweb.phy.anl.gov/EIC/juggler/-/blob/bf366a35b480cda6c610b0dd6a4d4efcfd9a8e03/JugReco/src/components/ClusterRecoCoG.cpp


        app->SetDefaultParameter("HCAL:HcalEndcapPClusters:samplingFraction",             m_sampFrac);
        app->SetDefaultParameter("HCAL:HcalEndcapPClusters:logWeightBase",  m_logWeightBase);
        app->SetDefaultParameter("HCAL:HcalEndcapPClusters:depthCorrection",     m_depthCorrection);
        app->SetDefaultParameter("HCAL:HcalEndcapPClusters:input_simhit_tag", m_input_simhit_tag);
        app->SetDefaultParameter("HCAL:HcalEndcapPClusters:input_protoclust_tag", m_input_protoclust_tag);
        app->SetDefaultParameter("HCAL:HcalEndcapPClusters:energyWeight",   m_energyWeight);
        app->SetDefaultParameter("HCAL:HcalEndcapPClusters:moduleDimZName",   m_moduleDimZName);
        app->SetDefaultParameter("HCAL:HcalEndcapPClusters:enableEtaBounds",   m_enableEtaBounds);

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
        event->Insert(m_outputAssociations, "HcalEndcapPClusterAssociations");
        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};
