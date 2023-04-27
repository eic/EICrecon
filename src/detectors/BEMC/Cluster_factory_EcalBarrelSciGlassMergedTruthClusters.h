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



class Cluster_factory_EcalBarrelSciGlassMergedTruthClusters : public JMultifactory, CalorimeterClusterMerger {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_EcalBarrelSciGlassMergedTruthClusters(){
        DeclarePodioOutput<edm4eic::Cluster>("EcalBarrelSciGlassMergedTruthClusters");
        DeclarePodioOutput<edm4eic::MCRecoClusterParticleAssociation>("EcalBarrelMergedClusterAssociations");
        // TODO: NWB: Inconsistent collection naming
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = japp; // GetApplication(); // TODO: NWB: FIXME after JANA2 v2.1.1
        m_log = app->GetService<Log_service>()->logger("EcalBarrelSciGlassMergedTruthClusters");

        //-------- Configuration Parameters ------------
        m_input_tag="EcalBarrelSciGlassTruthClusters";
        m_inputAssociations_tag="EcalBarrelSciGlassTruthClusterAssociations";

        app->SetDefaultParameter("BEMC:EcalBarrelMergedSciGlassTruthClusters:input_tag", m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("BEMC:EcalBarrelMergedSciGlassTruthClusters:inputAssociations_tag", m_inputAssociations_tag);

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
        SetData("EcalBarrelSciGlassMergedTruthClusters", m_outputClusters);
        SetData("EcalBarrelMergedClusterAssociations", m_outputAssociations);

        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }

private:
    // Name of input data type (collection)
    std::string              m_input_tag;
    std::string              m_inputAssociations_tag;
};
