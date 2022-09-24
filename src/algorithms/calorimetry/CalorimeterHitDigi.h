// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _CalorimeterHitDigi_h_
#define _CalorimeterHitDigi_h_

#include <random>

#include <services/geometry/dd4hep/JDD4hep_service.h>

#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <spdlog/spdlog.h>

class CalorimeterHitDigi {

    // Insert any member variables here

public:
    CalorimeterHitDigi() = default;
    ~CalorimeterHitDigi(){for( auto h : rawhits ) delete h;} // better to use smart pointer?
    virtual void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
    virtual void AlgorithmChangeRun() ;
    virtual void AlgorithmProcess() ;

    //-------- Configuration Parameters ------------
    //instantiate new spdlog logger
    std::shared_ptr<spdlog::logger> m_logger;

    // Name of input data type (collection)
    std::string              m_input_tag;

    // additional smearing resolutions
    std::vector<double>      u_eRes;
    double                   m_tRes;

    // digitization settings
    unsigned int             m_capADC;
    double                   m_dyRangeADC;
    unsigned int             m_pedMeanADC;
    double                   m_pedSigmaADC;
    double                   m_resolutionTDC;
    double                   m_corrMeanScale;

    // signal sums
    std::vector<std::string> u_fields;
    std::vector<int>         u_refs;
    std::string              m_geoSvcName;
    std::string              m_readout;

    // This may be used to declare the data members as JANA configuration parameters.
    // This should compile OK even without JANA so long as you don't try using it.
    // To use it, do something like the following:
    //
    //    mycalohitdigi->SetJANAConfigParameters( japp, "BEMC");
    //
    // The above will register config. parameters like: "BEMC:tag".
    // The configuration parameter members of this class should be set to thier
    // defaults *before* calling this.
    //-----------------------------------------------

    // unitless counterparts of inputs
    double           dyRangeADC{0}, stepTDC{0}, tRes{0}, eRes[3] = {0., 0., 0.};
    //Rndm::Numbers    m_normDist;
    std::shared_ptr<JDD4hep_service> m_geoSvc;
    uint64_t         id_mask{0}, ref_mask{0};

    // inputs/outputs
    std::vector<const edm4hep::SimCalorimeterHit*> simhits;
    std::vector<edm4hep::RawCalorimeterHit*> rawhits;

private:
    std::default_random_engine generator; // TODO: need something more appropriate here
    std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

    void single_hits_digi();
    void signal_sum_digi();
};

#endif // _CalorimeterHitDigi_h_
