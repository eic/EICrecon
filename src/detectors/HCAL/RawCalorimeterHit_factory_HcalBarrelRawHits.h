// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _RawCalorimeterHit_factory_HcalBarrelRawHits_h_
#define _RawCalorimeterHit_factory_HcalBarrelRawHits_h_

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


class RawCalorimeterHit_factory_HcalBarrelRawHits : public JFactoryT<edm4hep::RawCalorimeterHit>, CalorimeterHitDigi {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_HcalBarrelRawHits() {
        SetTag("HcalBarrelRawHits");
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        m_input_tag = "HcalBarrelHits";
        u_eRes = {};
        m_tRes = 0.0 * ns;
        m_capADC = 8096;
        m_dyRangeADC = 50 * GeV;
        m_pedMeanADC = 10;
        m_pedSigmaADC = 2.0;
        m_resolutionTDC = 1.0 * nanosecond;
        m_corrMeanScale = 1.0;
        u_fields={};
        u_refs={};
        m_geoSvcName = "ActsGeometryProvider";
        m_readout = "";
        m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        // This is another option for exposing the data members as JANA configuration parameters.
//        app->SetDefaultParameter("HCAL:tag",              m_input_tag);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:energyResolutions",u_eRes);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:timeResolution",   m_tRes);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:signalSumFields",  u_fields);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:fieldRefNumbers",  u_refs);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("HCAL:HcalBarrelRawHits:readoutClass",     m_readout);

        // Call Init for generic algorithm
        std::string tag=this->GetTag();
        std::shared_ptr<spdlog::logger> m_log = app->GetService<Log_service>()->logger(tag);

        // Get log level from user parameter or default
        std::string log_level_str = "info";
        auto pm = app->GetJParameterManager();
        pm->SetDefaultParameter(tag + ":LogLevel", log_level_str, "verbosity: trace, debug, info, warn, err, critical, off");
        m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
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

#endif // _RawCalorimeterHit_factory_HcalBarrelRawHits_h_
