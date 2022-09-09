

#ifndef _CalorimeterTruthClustering_h_
#define _CalorimeterTruthClustering_h_

#include <random>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4eic/ProtoCluster.h>
#include <edm4eic/MutableProtoCluster.h>


using namespace dd4hep;

class CalorimeterTruthClustering {

    // Insert any member variables here

public:
    CalorimeterTruthClustering() = default;
    ~CalorimeterTruthClustering(){} // better to use smart pointer?
    virtual void AlgorithmInit() ;
    virtual void AlgorithmChangeRun() ;
    virtual void AlgorithmProcess() ;

    //-------- Configuration Parameters ------------
    // Name of input data type (collection)
    std::string              m_inputHit_tag;
    std::string              m_inputMCHit_tag;


  std::vector<const edm4eic::CalorimeterHit*> m_inputHits;//{"inputHits", Gaudi::DataHandle::Reader, this};
  std::vector<const edm4hep::SimCalorimeterHit*> m_mcHits;//{"mcHits", Gaudi::DataHandle::Reader, this};

  std::vector<edm4eic::ProtoCluster*> m_outputProtoClusters;//{"outputProtoClusters", Gaudi::DataHandle::Writer, this};

private:

};

#endif //_CalorimeterTruthClustering_h_