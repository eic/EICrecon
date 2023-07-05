// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <edm4eic/ProtoClusterCollection.h>

#include <services/io/podio/JFactoryPodioT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterTruthClustering.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>


class ProtoCluster_factory_HcalBarrelTruthProtoClusters : public eicrecon::JFactoryPodioT<edm4eic::ProtoCluster>, CalorimeterTruthClustering {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_HcalBarrelTruthProtoClusters(){
        SetTag("HcalBarrelTruthProtoClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_inputHit_tag="HcalBarrelRecHits";
        m_inputMCHit_tag="HcalBarrelHits";

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

    std::shared_ptr<spdlog::logger> m_log;
};
