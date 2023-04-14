
#pragma once

#include <services/io/podio/JFactoryPodioT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_HcalEndcapNRecHits : public eicrecon::JFactoryPodioT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_HcalEndcapNRecHits(){
        SetTag("HcalEndcapNRecHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_input_tag = "HcalEndcapNRawHits";

        // digitization settings, must be consistent with digi class
        m_capADC=1024;//{this, "capacityADC", 8096};
        m_dyRangeADC=3.6 * dd4hep::MeV;//{this, "dynamicRangeADC", 100. * dd4hep::MeV};
        m_pedMeanADC=20;//{this, "pedestalMean", 400};
        m_pedSigmaADC=0.3;//{this, "pedestalSigma", 3.2};
        m_resolutionTDC=10 * dd4hep::picosecond;//{this, "resolutionTDC", 10 * ps};

        // zero suppression values
        m_thresholdFactor=4.0;//{this, "thresholdFactor", 0.0};
        m_thresholdValue=1.0;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=0.998;//{this, "samplingFraction", 1.0};

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="HcalEndcapNHits";  // from ATHENA's reconstruction.py
        m_layerField="";              // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="";             // from ATHENA's reconstruction.py

        m_localDetElement="";         // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={};          // from ATHENA's reconstruction.py (i.e. not defined there)

//        app->SetDefaultParameter("EHCAL:tag",              m_input_tag);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:readout",          m_readout);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("EHCAL:HcalEndcapNRecHits:localDetFields",   u_localDetFields);
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

};
