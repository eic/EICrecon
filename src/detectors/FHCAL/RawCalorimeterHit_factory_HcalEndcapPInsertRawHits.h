// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <JANA/JEvent.h>
#include <extensions/jana/JChainFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterHitDigi.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>



class RawCalorimeterHit_factory_HcalEndcapPInsertRawHits : public JChainFactoryT<edm4hep::RawCalorimeterHit>, CalorimeterHitDigi {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_HcalEndcapPInsertRawHits(std::vector<std::string> default_input_tags)
    : JChainFactoryT<edm4hep::RawCalorimeterHit>(std::move(default_input_tags)) {
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override {
        InitDataTags(GetPluginName() + ":" + GetTag());

        auto app = GetApplication();

        // random seed
        m_seed=0x670C84AE;

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        u_eRes = {};
        m_tRes = 0.0 * dd4hep::ns;
        m_capADC = 32768;
        m_dyRangeADC = 200 * dd4hep::MeV;
        m_pedMeanADC = 400;
        m_pedSigmaADC = 10;
        m_resolutionTDC = 10 * dd4hep::picosecond;
        m_corrMeanScale = 1.0;
        u_fields={};
        m_geoSvcName = "ActsGeometryProvider";
        m_readout = "";
        m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        // This is another option for exposing the data members as JANA configuration parameters.
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRawHits:energyResolutions",u_eRes);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRawHits:timeResolution",   m_tRes);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRawHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRawHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRawHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRawHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRawHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRawHits:scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRawHits:signalSumFields",  u_fields);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRawHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRawHits:readoutClass",     m_readout);

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
