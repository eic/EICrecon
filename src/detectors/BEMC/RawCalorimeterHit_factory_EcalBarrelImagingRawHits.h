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



class RawCalorimeterHit_factory_EcalBarrelImagingRawHits : public eicrecon::JFactoryPodioT<edm4hep::RawCalorimeterHit>, CalorimeterHitDigi {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_EcalBarrelImagingRawHits() {
        SetTag("EcalBarrelImagingRawHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        m_input_tag = "EcalBarrelImagingHits";
        u_eRes = {0.0 * sqrt(dd4hep::GeV), 0.02, 0.0 * dd4hep::GeV};
        m_tRes = 0.0 * dd4hep::ns;
        m_capADC = 8192;
        m_dyRangeADC = 3 * dd4hep::MeV;
        m_pedMeanADC = 100;
        m_pedSigmaADC = 14;
        m_resolutionTDC = 10 * dd4hep::picosecond;
        m_corrMeanScale = 1.0;
        m_geoSvcName = "ActsGeometryProvider";
        m_readout = "";
        m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        // This is another option for exposing the data members as JANA configuration parameters.
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:input_tag", m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:energyResolutions",u_eRes);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:timeResolution",   m_tRes);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:signalSumFields",  u_fields);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:fieldRefNumbers",  u_refs);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRawHits:readoutClass",     m_readout);

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
