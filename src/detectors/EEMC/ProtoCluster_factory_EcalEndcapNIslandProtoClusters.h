// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _ProtoCLuster_factory_EcalEndcapNIslandProtoClusters_h_
#define _ProtoCLuster_factory_EcalEndcapNIslandProtoClusters_h_

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterIslandCluster.h>

class ProtoCluster_factory_EcalEndcapNIslandProtoClusters : public JFactoryT<eicd::ProtoCluster>, CalorimeterIslandCluster {

public:
    //------------------------------------------
    // Constructor
    ProtoCluster_factory_EcalEndcapNIslandProtoClusters(){
        SetTag("EcalEndcapNIslandProtoClusters");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_input_tag = "EcalEndcapNRecHits";

        bool m_splitCluster=true;//{this, "splitCluster", true};
        double m_minClusterHitEdep=0.;//{this, "minClusterHitEdep", 0.};
        double m_minClusterCenterEdep=50.0 * MeV;//{this, "minClusterCenterEdep", 50.0 * MeV};


        // neighbour checking distances
        double m_sectorDist=5.0 * cm;//{this, "sectorDist", 5.0 * cm};
        std::vector<double> u_localDistXY={};//{this, "localDistXY", {}};
        std::vector<double> u_localDistXZ={};//{this, "localDistXZ", {}};
        std::vector<double> u_localDistYZ={};//{this, "localDistYZ", {}};
        std::vector<double> u_globalDistRPhi={};//{this, "globalDistRPhi", {}};
        std::vector<double> u_globalDistEtaPhi={};//{this, "globalDistEtaPhi", {}};
        std::vector<double> u_dimScaledLocalDistXY={1.8,1.8};//{this, "dimScaledLocalDistXY", {1.8, 1.8}};
  //     neighbor checking function
        std::function<edm4hep::Vector2f(const CaloHit&, const CaloHit&)> hitsDist;

  // unitless counterparts of the input parameters
        double minClusterHitEdep{0}, minClusterCenterEdep{0}, sectorDist{0};
        std::array<double, 2> neighbourDist = {0., 0.};

        app->SetDefaultParameter("EEMC:splitCluster",             m_splitCluster);
        app->SetDefaultParameter("EEMC:minClusterHitEdep",  m_minClusterHitEdep);
        app->SetDefaultParameter("EEMC:minClusterCenterEdep",     m_minClusterCenterEdep);
        //app->SetDefaultParameter("EEMC:inputHitCollection", m_inputHitCollection);
        //app->SetDefaultParameter("EEMC:outputProtoClusterCollection",    m_outputProtoCollection);
        app->SetDefaultParameter("EEMC:sectorDist",   m_sectorDist);
        app->SetDefaultParameter("EEMC:localDistXY",   u_localDistXY);
        app->SetDefaultParameter("EEMC:localDistXZ",   u_localDistXZ);
        app->SetDefaultParameter("EEMC:localDistYZ",  u_localDistYZ);
        app->SetDefaultParameter("EEMC:globalDistRPhi",    u_globalDistRPhi);
        app->SetDefaultParameter("EEMC:globalDistEtaPhi",    u_globalDistEtaPhi);
        app->SetDefaultParameter("EEMC:dimScaledLocalDistXY",    u_dimScaledLocalDistXY);
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
        hits = event->Get<eicd::CalorimeterHit>(m_input_tag);

        // Call Process for generic algorithm
        AlgorithmProcess();

        // Hand owner of algorithm objects over to JANA
        Set(protoClusters);
        protoClusters.clear(); // not really needed, but better to not leave dangling pointers around
    }
};

#endif // _ProtoCLuster_factory_EcalEndcapNIslandProtoClusters_h_
