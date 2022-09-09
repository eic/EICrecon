

#ifndef _CalorimeterHitReco_h_
#define _CalorimeterHitReco_h_

#include <random>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/RawCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/vector_utils.h>

using namespace dd4hep;

class CalorimeterHitReco {

    // Insert any member variables here

public:
    CalorimeterHitReco() = default;
    ~CalorimeterHitReco(){} // better to use smart pointer?
    virtual void AlgorithmInit() ;
    virtual void AlgorithmChangeRun() ;
    virtual void AlgorithmProcess() ;

    //-------- Configuration Parameters ------------
    // Name of input data type (collection)
    std::string              m_input_tag;

    double m_lUnit= 1.0 * dd4hep::mm;

  // digitization settings, must be consistent with digi class
  unsigned int m_capADC=8096;//{this, "capacityADC", 8096};
  double m_dyRangeADC=100. * MeV;//{this, "dynamicRangeADC", 100. * MeV};
  unsigned int m_pedMeanADC=400;//{this, "pedestalMean", 400};
  double m_pedSigmaADC=3.2;//{this, "pedestalSigma", 3.2};
  double m_resolutionTDC=10 * dd4hep::picosecond;//{this, "resolutionTDC", 10 * ps};

  // zero suppression values
  double m_thresholdFactor=0.0;//{this, "thresholdFactor", 0.0};
  double m_thresholdValue=0.0;//{this, "thresholdValue", 0.0};

  // energy correction with sampling fraction
  double m_sampFrac=1.0;//{this, "samplingFraction", 1.0};

  // unitless counterparts of the input parameters
  double dyRangeADC{0};
  double thresholdADC{0};
  double stepTDC{0};

    std::shared_ptr<JDD4hep_service> m_geoSvc;
  //DataHandle<edm4eic::RawCalorimeterHitCollection> m_inputHitCollection{"inputHitCollection", Gaudi::DataHandle::Reader,  this};
  //DataHandle<edm4eic::CalorimeterHitCollection> m_outputHitCollection{"outputHitCollection", Gaudi::DataHandle::Writer,   this};

  // geometry service to get ids, ignored if no names provided
  std::string m_geoSvcName="geoServiceName";
  std::string m_readout="readoutClass";
  std::string m_layerField="layerField";
  std::string m_sectorField="sectorField";

  dd4hep::BitFieldCoder* id_dec = nullptr;
  
  size_t sector_idx{0}, layer_idx{0};

  // name of detelment or fields to find the local detector (for global->local transform)
  // if nothing is provided, the lowest level DetElement (from cellID) will be used
  std::string m_localDetElement="";
  std::vector<std::string> u_localDetFields={};
  dd4hep::DetElement local;
  size_t local_mask = ~0;

    std::vector<edm4eic::CalorimeterHit*> hits;
    std::vector<const edm4hep::RawCalorimeterHit*> rawhits;

private:
    //std::default_random_engine generator; // TODO: need something more appropriate here
    //std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1
};

#endif // _CalorimeterHitReco_h_