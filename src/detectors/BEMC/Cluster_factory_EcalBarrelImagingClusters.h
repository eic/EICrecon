// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <JANA/JMultifactory.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/ImagingClusterReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class Cluster_factory_EcalBarrelImagingClusters : public JMultifactory, ImagingClusterReco {

public:

    std::string m_input_simhit_tag;
    std::string m_input_protoclust_tag;

    //------------------------------------------
    // Constructor
    Cluster_factory_EcalBarrelImagingClusters(){
        DeclarePodioOutput<edm4eic::Cluster>("EcalBarrelImagingClusters");
        DeclarePodioOutput<edm4eic::MCRecoClusterParticleAssociation>("EcalBarrelImagingClusterAssociations");
        DeclarePodioOutput<edm4eic::Cluster>("EcalBarrelImagingLayers");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = japp; // GetApplication(); // TODO: NWB: FIXME after JANA2 v2.1.1
        m_log = app->GetService<Log_service>()->logger("EcalBarrelImagingClusters");

        //-------- Configuration Parameters ------------
        m_input_simhit_tag="EcalBarrelImagingHits";
        m_input_protoclust_tag="EcalBarrelImagingProtoClusters";
        m_trackStopLayer = 6;

        app->SetDefaultParameter("BEMC:EcalBarrelImagingClusters:input_protoclust_tag",        m_input_protoclust_tag, "Name of input collection to use");
        app->SetDefaultParameter("BEMC:EcalBarrelImagingClusters:trackStopLayer",  m_trackStopLayer);

        initialize();
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override{

        // Prefill inputs
        m_mcHits=event->Get<edm4hep::SimCalorimeterHit>(m_input_simhit_tag);
        m_inputProtoClusters=event->Get<edm4eic::ProtoCluster>(m_input_protoclust_tag);

        // Call Process for generic algorithm
        execute();

        // Hand owner of algorithm objects over to JANA
        SetData("EcalBarrelImagingClusters", m_outputClusters);
        SetData("EcalBarrelImagingClusterAssociations", m_outputAssociations);
        SetData("EcalBarrelImagingLayers", m_outputLayers);

        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
        m_outputLayers.clear();
    }
};
