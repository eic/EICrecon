// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <JANA/JEvent.h>
#include <services/io/podio/JFactoryPodioT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterHitDigi.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class RawCalorimeterHit_factory_HcalBarrelRawHits : public eicrecon::JFactoryPodioT<edm4hep::RawCalorimeterHit>, CalorimeterHitDigi {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_HcalBarrelRawHits() {
        SetTag("HcalBarrelRawHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        m_input_tag = "HcalBarrelHits";
        u_eRes = {};
        m_tRes = 0.0 * dd4hep::ns;
        m_capADC = 65536;
        m_dyRangeADC = 1.0 * dd4hep::GeV;
        m_pedMeanADC = 10;
        m_pedSigmaADC = 2.0;
        m_capTime = 100 ; // given in ns, 4 samples in HGCROC
        m_resolutionTDC = 1.0 * dd4hep::picosecond;
        m_corrMeanScale = 1.0;
        u_fields={};
        u_refs={};
        m_geoSvcName = "geoServiceName";
        m_readout = "HcalBarrelHits";
        m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        // This is another option for exposing the data members as JANA configuration parameters.
//        app->SetDefaultParameter("BHCAL:tag",              m_input_tag);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:energyResolutions",u_eRes);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:timeResolution",   m_tRes);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:signalSumFields",  u_fields);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:fieldRefNumbers",  u_refs);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("BHCAL:HcalBarrelRawHits:readoutClass",     m_readout);

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
