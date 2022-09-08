// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _Clusters_factory_EcalEndcapNClusters_h_
#define _Clusters_factory_EcalEndcapNClusters_h_

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterClusterRecoCoG.h>



class Cluster_factory_EcalEndcapNClusters : public JFactoryT<eicd::Cluster>, CalorimeterClusterRecoCoG {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_EcalEndcapNClusters(){
        SetTag("EcalEndcapNClusters");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        //-------- Configuration Parameters ------------
        m_input_simhit_tag="EcalEndcapNHits";
        m_input_protoclust_tag="EcalEndcapNTruthProtoClusters";
    
        m_sampFrac=1.0;//{this, "samplingFraction", 1.0};
        m_logWeightBase=3.6;//{this, "logWeightBase", 3.6};
        m_depthCorrection=0.0;//{this, "depthCorrection", 0.0};
        m_energyWeight="log";//{this, "energyWeight", "log"};
        m_moduleDimZName="";//{this, "moduleDimZName", ""};
        // Constrain the cluster position eta to be within
        // the eta of the contributing hits. This is useful to avoid edge effects
        // for endcaps.
        m_enableEtaBounds=false;//{this, "enableEtaBounds", false};


        app->SetDefaultParameter("EEMC:samplingFraction",             m_sampFrac);
        app->SetDefaultParameter("EEMC:logWeightBase",  m_logWeightBase);
        app->SetDefaultParameter("EEMC:depthCorrection",     m_depthCorrection);
        //app->SetDefaultParameter("EEMC:inputHitCollection", m_inputHitCollection);
        //app->SetDefaultParameter("EEMC:outputProtoClusterCollection",    m_outputProtoCollection);
        app->SetDefaultParameter("EEMC:energyWeight",   m_energyWeight);
        app->SetDefaultParameter("EEMC:moduleDimZName",   m_moduleDimZName);
        app->SetDefaultParameter("EEMC:enableEtaBounds",   m_enableEtaBounds);

        m_geoSvc = app->template GetService<JDD4hep_service>();

        AlgorithmInit();
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
        m_inputProto=event->Get<eicd::ProtoCluster>(m_input_protoclust_tag); 

        // Call Process for generic algorithm
        AlgorithmProcess();


        //outputs

        // Hand owner of algorithm objects over to JANA
        Set(m_outputClusters);
        event->Insert(m_outputAssociations, "EcalEndcapMCRecoClusterParticleAssociation");
        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};

#endif // _Clusters_factory_EcalEndcapNClusters_h_
