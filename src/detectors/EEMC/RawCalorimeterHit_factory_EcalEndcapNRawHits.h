// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _RawCalorimeterHit_factory_EcalEndcapNRawHits_h_
#define _RawCalorimeterHit_factory_EcalEndcapNRawHits_h_

#include <random>

#include <JANA/JEvent.h>
#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterHitDigi.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
using namespace dd4hep;


class RawCalorimeterHit_factory_EcalEndcapNRawHits : public JFactoryT<edm4hep::RawCalorimeterHit>, CalorimeterHitDigi {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_EcalEndcapNRawHits() {
        SetTag("EcalEndcapNRawHits");
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        m_input_tag = "EcalEndcapNHits";
        m_tRes = 0.0 * ns;
        m_tRes = 0.0 * ns;
        m_capADC = 8096;
        m_dyRangeADC = 100 * MeV;
        m_pedMeanADC = 400;
        m_pedSigmaADC = 3.2;
        m_resolutionTDC = 10 * picosecond;
        m_corrMeanScale = 1.0;
        m_geoSvcName = "ActsGeometryProvider";
        m_readout = "";
        m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        // This is another option for exposing the data members as JANA configuration parameters.
//        app->SetDefaultParameter("EEMC:tag",              m_input_tag);
        app->SetDefaultParameter("EEMC:energyResolutions",u_eRes);
        app->SetDefaultParameter("EEMC:timeResolution",   m_tRes);
        app->SetDefaultParameter("EEMC:capacityADC",      m_capADC);
        app->SetDefaultParameter("EEMC:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("EEMC:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("EEMC:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("EEMC:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("EEMC:scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter("EEMC:signalSumFields",  u_fields);
        app->SetDefaultParameter("EEMC:fieldRefNumbers",  u_refs);
        app->SetDefaultParameter("EEMC:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("EEMC:readoutClass",     m_readout);

        // Call Init for generic algorithm
        AlgorithmInit();
    }

    //------------------------------------------
    // ChangeRun
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override {
        AlgorithmChangeRun();
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override {
        // Prefill inputs
        simhits = event->Get<edm4hep::SimCalorimeterHit>(m_input_tag);

        // Call Process for generic algorithm
        AlgorithmProcess();

        // Hand owner of algorithm objects over to JANA
        Set(rawhits);
        rawhits.clear(); // not really needed, but better to not leave dangling pointers around
    }

};

#endif // _RawCalorimeterHit_factory_EcalEndcapNRawHits_h_
