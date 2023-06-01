// Copyright 2022, Thomas Britton
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <services/io/podio/JFactoryPodioT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/ImagingClusterReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class Cluster_factory_EcalBarrelImagingClusters : public eicrecon::JFactoryPodioT<edm4eic::Cluster>, ImagingClusterReco {

public:

    std::string m_input_simhit_tag;
    std::string m_input_protoclust_tag;

    //------------------------------------------
    // Constructor
    Cluster_factory_EcalBarrelImagingClusters(){
        SetTag("EcalBarrelImagingClusters");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
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
        Set(m_outputClusters);
        event->Insert(m_outputAssociations, "EcalBarrelImagingClusterAssociations");
        event->Insert(m_outputLayers, "EcalBarrelImagingLayers");
        m_outputClusters.clear(); // not really needed, but better to not leave dangling pointers around
        m_outputAssociations.clear();
        m_outputLayers.clear();
    }
};
