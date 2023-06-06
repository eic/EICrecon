// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <random>

#include <services/io/podio/JFactoryPodioT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/TruthEnergyPositionClusterMerger.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>


class Cluster_factory_EcalBarrelImagingMergedClusters : public eicrecon::JFactoryPodioT<edm4eic::Cluster>, TruthEnergyPositionClusterMerger {

public:
    //------------------------------------------
    // Constructor
    Cluster_factory_EcalBarrelImagingMergedClusters(){
        SetTag("EcalBarrelImagingMergedClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        //-------- Configuration Parameters ------------
        m_inputMCParticles_tag = "MCParticles";
        m_energyClusters_tag = "EcalBarrelScFiClusters";
        m_energyAssociation_tag = "EcalBarrelScFiClusterAssociations";
        m_positionClusters_tag = "EcalBarrelImagingClusters";
        m_positionAssociations_tag = "EcalBarrelImagingClusterAssociations";

        app->SetDefaultParameter("BEMC:EcalBarrelImagingMergedClusters:inputMCParticles_tag", m_inputMCParticles_tag);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingMergedClusters:energyClusters_tag", m_energyClusters_tag);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingMergedClusters:energyAssociation_tag", m_energyAssociation_tag);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingMergedClusters:positionClusters_tag", m_positionClusters_tag);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingMergedClusters:positionAssociations_tag", m_positionAssociations_tag);

        initialize();
    }


    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override{

        // Prefill inputs
        m_inputMCParticles     = event->Get<edm4hep::MCParticle>(m_inputMCParticles_tag);;
        m_energyClusters       = event->Get<edm4eic::Cluster>(m_energyClusters_tag);;
        m_energyAssociations   = event->Get<edm4eic::MCRecoClusterParticleAssociation>(m_energyAssociation_tag);;
        m_positionClusters     = event->Get<edm4eic::Cluster>(m_positionClusters_tag);;
        m_positionAssociations = event->Get<edm4eic::MCRecoClusterParticleAssociation>(m_positionAssociations_tag);;

        // Call Process for generic algorithm
        execute();

        // Hand owner of algorithm objects over to JANA
        Set(m_outputClusters);
        event->Insert(m_outputAssociations, "EcalBarrelImagingMergedClustersAssoc");
        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
    }

private:
    // Name of input data type (collection)
    std::string m_inputMCParticles_tag;
    std::string m_energyClusters_tag;
    std::string m_energyAssociation_tag;
    std::string m_positionClusters_tag;
    std::string m_positionAssociations_tag;
};
