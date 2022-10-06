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

using namespace dd4hep;


class RawCalorimeterHit_factory_EcalBarrelImagingRawHits : public JFactoryT<edm4hep::RawCalorimeterHit>, CalorimeterHitDigi {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_EcalBarrelImagingRawHits() {
        SetTag("EcalBarrelImagingRawHits");
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        m_input_tag = "EcalBarrelHits";
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
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:energyResolutions",u_eRes);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:timeResolution",   m_tRes);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:signalSumFields",  u_fields);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:fieldRefNumbers",  u_refs);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingHits:readoutClass",     m_readout);

        // Call Init for generic algorithm
        std::string tag=this->GetTag();
        std::shared_ptr<spdlog::logger> logger = app->GetService<Log_service>()->logger(tag);

        // Get log level from user parameter or default
        std::string log_level_str = "info";
        auto pm = app->GetJParameterManager();
        pm->SetDefaultParameter(tag + ":LogLevel", log_level_str, "verbosity: trace, debug, info, warn, err, critical, off");
        logger->set_level(eicrecon::ParseLogLevel(log_level_str));
        AlgorithmInit(logger);
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

