

#ifndef _CalorimeterClusterRecoCoG_h_
#define _CalorimeterClusterRecoCoG_h_

#include <random>

#include <services/geometry/dd4hep/JDD4hep_service.h>


using namespace dd4hep;

class CalorimeterClusterRecoCoG {

    // Insert any member variables here

public:
    CalorimeterClusterRecoCoG() = default;
    ~CalorimeterClusterRecoCoG(){} // better to use smart pointer?
    virtual void AlgorithmInit() ;
    virtual void AlgorithmChangeRun() ;
    virtual void AlgorithmProcess() ;

    //-------- Configuration Parameters ------------
    std::string m_input_tag;
    
    double m_sampFrac;//{this, "samplingFraction", 1.0};
    double m_logWeightBase;//{this, "logWeightBase", 3.6};
    double m_depthCorrection;//{this, "depthCorrection", 0.0};
    std::string m_energyWeight;//{this, "energyWeight", "log"};
    std::string m_moduleDimZName;//{this, "moduleDimZName", ""};
    // Constrain the cluster position eta to be within
    // the eta of the contributing hits. This is useful to avoid edge effects
    // for endcaps.
    bool m_enableEtaBounds;//{this, "enableEtaBounds", false};

    std::shared_ptr<JDD4hep_service> m_geoSvc;

    double m_depthCorr{0};
    std::function<double(double, double, double, int)> weightFunc;

    template<typename T> // n.b. OutputType should be JApplication
    void SetJANAConfigParameters(T *app, const std::string& prefix){
        app->SetDefaultParameter(prefix+":tag",             m_input_tag, "tag/collection name for edm4hep::SimCalorimeterHit objects to use");
        app->SetDefaultParameter(prefix+":samplingFraction",u_eRes);
        app->SetDefaultParameter(prefix+":logWeightBase",  m_tRes);
        app->SetDefaultParameter(prefix+":depthCorrection",     m_capADC);
        app->SetDefaultParameter(prefix+":energyWeight", m_dyRangeADC);
        app->SetDefaultParameter(prefix+":moduleDimZName",    m_pedMeanADC);
        app->SetDefaultParameter(prefix+":enableEtaBounds",   m_pedSigmaADC);
        m_geoSvc = app->template GetService<JDD4hep_service>(); // TODO: implement named geometry service?
    }


  //inputs EcalEndcapNTruthProtoClusters AND EcalEndcapNHits

    DataHandle<eicd::ProtoClusterCollection> m_inputProto{"inputProtoClusterCollection", Gaudi::DataHandle::Reader, this};
    DataHandle<eicd::ClusterCollection> m_outputClusters{"outputClusterCollection", Gaudi::DataHandle::Writer, this};

    // Collection for MC hits when running on MC
    Gaudi::Property<std::string> m_mcHits{this, "mcHits", ""};
    // Optional handle to MC hits
    std::unique_ptr<DataHandle<edm4hep::SimCalorimeterHitCollection>> m_mcHits_ptr;

    // Collection for associations when running on MC
    Gaudi::Property<std::string> m_outputAssociations{this, "outputAssociations", ""};
    // Optional handle to MC hits
    std::unique_ptr<DataHandle<eicd::MCRecoClusterParticleAssociationCollection>> m_outputAssociations_ptr;
    
    

private:

};

#endif // _CalorimeterClusterRecoCoG_h_