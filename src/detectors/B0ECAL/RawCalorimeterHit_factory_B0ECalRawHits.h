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



class RawCalorimeterHit_factory_B0ECalRawHits : public eicrecon::JFactoryPodioT<edm4hep::RawCalorimeterHit>, CalorimeterHitDigi {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_B0ECalRawHits() {
        SetTag("B0ECalRawHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        m_input_tag = "B0ECalHits";
        u_eRes = {0.0 * sqrt(dd4hep::GeV), 0.02, 0.0 * dd4hep::GeV};
        m_tRes = 0.0 * dd4hep::ns;
        m_capADC = 16384;
        m_dyRangeADC = 20 * dd4hep::GeV;
        m_pedMeanADC = 100;
        m_pedSigmaADC = 1;
        m_resolutionTDC = 1e-11;
        m_corrMeanScale = 1.0;
        u_fields={};
        u_refs={};
        m_geoSvcName = "";
        m_readout = "";

        m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?


        // This is another option for exposing the data members as JANA configuration parameters.
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:energyResolutions",u_eRes);
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:timeResolution",   m_tRes);
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:signalSumFields",  u_fields);
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:fieldRefNumbers",  u_refs);
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("B0ECAL:B0ECalRawHits:readoutClass",     m_readout);

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
