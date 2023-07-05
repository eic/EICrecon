// Copyright 2023, Friederike Bock
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _TruthCluster_factory_LFHCALTruthProtoClusters_h_
#define _TruthCluster_factory_LFHCALTruthProtoClusters_h_

#include <random>

#include <services/io/podio/JFactoryPodioT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterTruthClustering.h>



class ProtoCluster_factory_LFHCALTruthProtoClusters : public eicrecon::JFactoryPodioT<edm4eic::ProtoCluster>, CalorimeterTruthClustering {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_LFHCALTruthProtoClusters(){
        SetTag("LFHCALTruthProtoClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_inputHit_tag="LFHCALRecHits";
        m_inputMCHit_tag="LFHCALHits";

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
        // Get input collection
        auto hits_coll = static_cast<const edm4eic::CalorimeterHitCollection*>(event->GetCollectionBase(m_inputHit_tag));
        auto sim_coll = static_cast<const edm4hep::SimCalorimeterHitCollection*>(event->GetCollectionBase(m_inputMCHit_tag));

        // Call Process for generic algorithm
        auto protoclust_coll = AlgorithmProcess(*hits_coll, *sim_coll);

        // Hand algorithm objects over to JANA
        SetCollection(std::move(protoclust_coll));
    }
private:
    // Name of input data type (collection)
    std::string              m_inputHit_tag;
    std::string              m_inputMCHit_tag;
};

#endif // _ProtoCLuster_factory_IslandProtoClusters_h_
