// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _JFactory_EcalBarrelRawCalorimeterHit_h_
#define _JFactory_EcalBarrelRawCalorimeterHit_h_

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>

#include "EcalBarrelRawCalorimeterHit.h"

class JFactory_EcalBarrelRawCalorimeterHit : public JFactoryT<EcalBarrelRawCalorimeterHit> {

    // Insert any member variables here

public:
    JFactory_EcalBarrelRawCalorimeterHit();
    void Init() override;
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override;
    void Process(const std::shared_ptr<const JEvent> &event) override;

    //-------- Configuration Parameters ------------
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
    //-----------------------------------------------

    // unitless counterparts of inputs
    double           dyRangeADC{0}, stepTDC{0}, tRes{0}, eRes[3] = {0., 0., 0.};
    //Rndm::Numbers    m_normDist;
    std::shared_ptr<JDD4hep_service> m_geoSvc;
    uint64_t         id_mask{0}, ref_mask{0};

private:
    std::default_random_engine generator; // TODO: need something more appropriate here
    std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

    void single_hits_digi( const std::shared_ptr<const JEvent> &event );
    void signal_sum_digi( const std::shared_ptr<const JEvent> &event );
};

#endif // _JFactory_EcalBarrelRawCalorimeterHit_h_
