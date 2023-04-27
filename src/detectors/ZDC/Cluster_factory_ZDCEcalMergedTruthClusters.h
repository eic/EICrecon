// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <random>

#include <JANA/JMultifactory.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterClusterMerger.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class Cluster_factory_ZDCEcalMergedTruthClusters : public JMultifactory, CalorimeterClusterMerger {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_ZDCEcalMergedTruthClusters(){
        DeclarePodioOutput<edm4eic::Cluster>("ZDCEcalMergedTruthClusters");
        DeclarePodioOutput<edm4eic::MCRecoClusterParticleAssociation>("ZDCEcalMergedTruthClusterAssociations");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = japp; // GetApplication(); // TODO: NWB: FIXME after JANA2 v2.1.1
        m_log = app->GetService<Log_service>()->logger("ZDCEcalMergedTruthClusters");

        //-------- Configuration Parameters ------------
        m_input_tag="ZDCEcalTruthClusters";
        m_inputAssociations_tag="ZDCEcalTruthClusterAssociations";

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
        m_inputClusters=event->Get<edm4eic::Cluster>(m_input_tag);
        m_inputAssociations=event->Get<edm4eic::MCRecoClusterParticleAssociation>(m_inputAssociations_tag);

        // Call Process for generic algorithm
        AlgorithmProcess();
        // Hand owner of algorithm objects over to JANA
        SetData("ZDCEcalMergedClusters", m_outputClusters);
        SetData("ZDCEcalMergedClusterAssociations", m_outputAssociations);

        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }

private:
    // Name of input data type (collection)
    std::string              m_input_tag;
    std::string              m_inputAssociations_tag;
};
