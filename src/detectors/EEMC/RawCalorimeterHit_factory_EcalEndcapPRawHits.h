// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <JANA/JEvent.h>
#include <services/io/podio/datamodel_glue.h>
#include <JANA/Podio/JFactoryPodioT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterHitDigi.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class RawCalorimeterHit_factory_EcalEndcapPRawHits : public JFactoryPodioT<edm4hep::RawCalorimeterHit>, CalorimeterHitDigi {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_EcalEndcapPRawHits() {
        SetTag("EcalEndcapPRawHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        m_input_tag = "EcalEndcapPHits";
        u_eRes = {3.16 * dd4hep::MeV, 1.5 * dd4hep::MeV, 0.0 * dd4hep::MeV};
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
//        app->SetDefaultParameter("EEMC:tag",              m_input_tag);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:energyResolutions",u_eRes);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:timeResolution",   m_tRes);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:signalSumFields",  u_fields);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:fieldRefNumbers",  u_refs);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("EEMC:EcalEndcapPRawHits:readoutClass",     m_readout);

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
