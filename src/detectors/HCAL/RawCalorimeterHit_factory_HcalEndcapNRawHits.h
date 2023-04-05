// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <JANA/JEvent.h>
#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterHitDigi.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class RawCalorimeterHit_factory_HcalEndcapNRawHits : public JFactoryT<edm4hep::RawCalorimeterHit>, CalorimeterHitDigi {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_HcalEndcapNRawHits() {
        SetTag("HcalEndcapNRawHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        m_input_tag = "HcalEndcapNHits";
        u_eRes = {};
        m_tRes = 0.0 * dd4hep::ns;
        m_capADC = 1024;
        m_dyRangeADC = 3.6 * dd4hep::MeV;
        m_pedMeanADC = 20;
        m_pedSigmaADC = 0.3;
        m_resolutionTDC = 10 * dd4hep::picosecond;
        m_corrMeanScale = 1.0;
        u_fields={};
        u_refs={};
        m_geoSvcName = "ActsGeometryProvider";
        m_readout = "";
        m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        // This is another option for exposing the data members as JANA configuration parameters.
//        app->SetDefaultParameter("HCAL:tag",              m_input_tag);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:energyResolutions",u_eRes);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:timeResolution",   m_tRes);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:signalSumFields",  u_fields);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:fieldRefNumbers",  u_refs);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("HCAL:HcalEndcapNRawHits:readoutClass",     m_readout);

        // Call Init for generic algorithm
        AlgorithmInit(m_log);
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
