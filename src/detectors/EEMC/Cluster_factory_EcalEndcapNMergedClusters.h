// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _Cluster_factory_EcalEndcapNMergedClusters_h_
#define _Cluster_factory_EcalEndcapNMergedClusters_h_

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterClusterMerger.h>



class Cluster_factory_EcalEndcapNMergedClusters : public JFactoryT<eicd::Cluster>, CalorimeterClusterMerger {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_EcalEndcapNMergedClusters(){
        SetTag("EcalEndcapNMergedClusters");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        //-------- Configuration Parameters ------------
        m_input_tag="EcalEndcapNClusters";
        m_inputAssociations_tag="EcalEndcapNClustersAssoc";

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
        m_inputClusters=event->Get<eicd::Cluster>(m_input_tag);
        m_inputAssociations=event->Get<eicd::MCRecoClusterParticleAssociation>(m_inputAssociations_tag); 

        // Call Process for generic algorithm
        AlgorithmProcess();

        //outputs
        // Hand owner of algorithm objects over to JANA
        Set(m_outputClusters);
        event->Insert(m_outputAssociations, "EcalEndcapNMergedClustersAssoc");
        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }
};

#endif // _Cluster_factory_EcalEndcapNMergedClusters_h_
