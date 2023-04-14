
#pragma once

#include <services/io/podio/JFactoryPodioT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_HcalBarrelRecHits : public eicrecon::JFactoryPodioT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_HcalBarrelRecHits(){
        SetTag("HcalBarrelRecHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_input_tag = "HcalBarrelRawHits";

        // digitization settings, must be consistent with digi class
        m_capADC=65536;
        m_dyRangeADC=1.0 * dd4hep::GeV;
        m_pedMeanADC=10;
        m_pedSigmaADC=2.0;
        m_resolutionTDC=1.0 * dd4hep::picosecond;

        // zero suppression values
        m_thresholdFactor=5.0;// from ATHENA's reconstruction.py
        m_thresholdValue=1.0;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=0.033;  // average, from sPHENIX simulations

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="HcalBarrelHits";
        m_layerField="tower";
        m_sectorField="sector";

        m_localDetElement="";
        u_localDetFields={};

//        app->SetDefaultParameter("BHCAL:tag",              m_input_tag);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:readout",          m_readout);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("BHCAL:HcalBarrelRecHits:localDetFields",   u_localDetFields);
        m_geoSvc = app->template GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        AlgorithmInit(m_log);
    }

    //------------------------------------------
    // ChangeRun
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override{
        AlgorithmChangeRun();
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override{
        // Prefill inputs
        rawhits = event->Get<edm4hep::RawCalorimeterHit>(m_input_tag);

        // Call Process for generic algorithm
        AlgorithmProcess();

        // Hand owner of algorithm objects over to JANA
        Set(hits);
        hits.clear(); // not really needed, but better to not leave dangling pointers around
    }

    std::shared_ptr<spdlog::logger> m_log;
};
