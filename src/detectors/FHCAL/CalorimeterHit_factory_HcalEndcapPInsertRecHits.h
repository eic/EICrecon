
#pragma once

#include <services/io/podio/JFactoryPodioT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_HcalEndcapPInsertRecHits : public eicrecon::JFactoryPodioT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_HcalEndcapPInsertRecHits(){
        SetTag("HcalEndcapPInsertRecHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_input_tag = "HcalEndcapPInsertRawHits";

        // digitization settings, must be consistent with digi class
        m_capADC=32768;//{this, "capacityADC", 8096};
        m_dyRangeADC=200. * dd4hep::MeV;//{this, "dynamicRangeADC", 100. * dd4hep::MeV};
        m_pedMeanADC=400;//{this, "pedestalMean", 400};
        m_pedSigmaADC=10.;//{this, "pedestalSigma", 3.2};
        m_resolutionTDC=10 * dd4hep::picosecond;//{this, "resolutionTDC", 10 * ps};

        // zero suppression values
        m_thresholdFactor=0.;//{this, "thresholdFactor", 0.0};
        m_thresholdValue=-100.;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=0.0098; // from standalone studies of hcal insert

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="HcalEndcapPInsertHits";  // from ATHENA's reconstruction.py
        m_layerField="";              // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="";             // from ATHENA's reconstruction.py

        m_localDetElement="";         // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={};          // from ATHENA's reconstruction.py (i.e. not defined there)

//        app->SetDefaultParameter("FHCAL:tag",              m_input_tag);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:readout",          m_readout);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertRecHits:localDetFields",   u_localDetFields);
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
