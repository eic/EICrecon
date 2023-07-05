
#pragma once

#include <edm4eic/CalorimeterHitCollection.h>

#include <services/io/podio/JFactoryPodioT.h>
#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_HcalEndcapPRecHits : public eicrecon::JFactoryPodioT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_HcalEndcapPRecHits(){
        SetTag("HcalEndcapPRecHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_input_tag = "HcalEndcapPRawHits";

        // digitization settings, must be consistent with digi class
        m_capADC=65536;
        m_dyRangeADC=1 * dd4hep::GeV; // based on LFHCal (with 10 tiles vs. 1 here)
        m_pedMeanADC=20;
        m_pedSigmaADC=0.8;
        m_resolutionTDC=10 * dd4hep::picosecond;

        // zero suppression values
        m_thresholdFactor=1.0;
        m_thresholdValue=3.0;

        // energy correction with sampling fraction
        m_sampFrac=0.033;

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="HcalEndcapPHits";  // from ATHENA's reconstruction.py
        m_layerField="";              // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="";             // from ATHENA's reconstruction.py

        m_localDetElement="";         // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={};          // from ATHENA's reconstruction.py (i.e. not defined there)

//        app->SetDefaultParameter("FHCAL:tag",              m_input_tag);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:readout",          m_readout);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("FHCAL:HcalEndcapPRecHits:localDetFields",   u_localDetFields);
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
        // Get input collection
        auto rawhits_coll = static_cast<const edm4hep::RawCalorimeterHitCollection*>(event->GetCollectionBase(m_input_tag));

        // Call Process for generic algorithm
        auto recohits_coll = AlgorithmProcess(*rawhits_coll);

        // Hand algorithm objects over to JANA
        SetCollection(std::move(recohits_coll));
    }

};
