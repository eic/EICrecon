// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JEvent.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHitCollection.h>

#include <extensions/jana/JChainFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterHitDigi.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class RawCalorimeterHit_factory_HcalEndcapNRawHits : public JChainFactoryT<edm4hep::RawCalorimeterHit>, CalorimeterHitDigi {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_HcalEndcapNRawHits(std::vector<std::string> default_input_tags)
    : JChainFactoryT<edm4hep::RawCalorimeterHit>(std::move(default_input_tags)) {
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override {
        InitDataTags(GetPluginName() + ":" + GetTag());

        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        u_eRes = {};
        m_tRes = 0.0 * dd4hep::ns;
        m_capADC = 1024;
        m_dyRangeADC = 3.6 * dd4hep::MeV;
        m_pedMeanADC = 20;
        m_pedSigmaADC = 0.3;
        m_resolutionTDC = 10 * dd4hep::picosecond;
        m_corrMeanScale = 1.0;
        u_fields={};
        m_geoSvcName = "ActsGeometryProvider";
        m_readout = "";
        m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        // This is another option for exposing the data members as JANA configuration parameters.
        app->SetDefaultParameter("EHCAL:HcalEndcapNRawHits:energyResolutions",u_eRes);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRawHits:timeResolution",   m_tRes);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRawHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRawHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRawHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRawHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRawHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRawHits:scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRawHits:signalSumFields",  u_fields);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRawHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRawHits:readoutClass",     m_readout);

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
        // Get input collection
        auto simhits_coll = static_cast<const edm4hep::SimCalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        // Call Process for generic algorithm
        auto rawhits_coll = AlgorithmProcess(*simhits_coll);

        // Hand algorithm objects over to JANA
        SetCollection(std::move(rawhits_coll));
    }

};
