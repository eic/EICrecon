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



class RawCalorimeterHit_factory_EcalEndcapPInsertRawHits : public eicrecon::JFactoryPodioT<edm4hep::RawCalorimeterHit>, CalorimeterHitDigi {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_EcalEndcapPInsertRawHits() {
        SetTag("EcalEndcapPInsertRawHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        m_input_tag = "EcalEndcapPInsertHits";
        u_eRes = {0.00316 * sqrt(dd4hep::GeV), 0.0015, 0.0 * dd4hep::GeV}; // (0.316% / sqrt(E)) \oplus 0.15%
        m_tRes = 0.0 * dd4hep::ns;
        m_capADC = 16384;
        m_dyRangeADC = 3 * dd4hep::GeV;
        m_pedMeanADC = 100;
        m_pedSigmaADC = 0.7;
        m_resolutionTDC = 10 * dd4hep::picosecond;
        m_corrMeanScale = 0.03;
        u_fields={};
        u_refs={1, 1};
        m_geoSvcName = "ActsGeometryProvider";
        m_readout = "";
        m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        // This is another option for exposing the data members as JANA configuration parameters.
//        app->SetDefaultParameter("FEMC:tag",              m_input_tag);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:energyResolutions",u_eRes);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:timeResolution",   m_tRes);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:signalSumFields",  u_fields);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:fieldRefNumbers",  u_refs);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRawHits:readoutClass",     m_readout);

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
