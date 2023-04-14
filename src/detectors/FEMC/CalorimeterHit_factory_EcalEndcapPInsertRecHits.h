
#pragma once

#include <services/io/podio/JFactoryPodioT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_EcalEndcapPInsertRecHits : public eicrecon::JFactoryPodioT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_EcalEndcapPInsertRecHits(){
        SetTag("EcalEndcapPInsertRecHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_input_tag = "EcalEndcapPInsertRawHits";

        // digitization settings, must be consistent with digi class
        m_capADC=16384;//{this, "capacityADC", 8096};
        m_dyRangeADC=3. * dd4hep::GeV;//{this, "dynamicRangeADC", 100. * dd4hep::MeV};
        m_pedMeanADC=100;//{this, "pedestalMean", 400};
        m_pedSigmaADC=0.7;//{this, "pedestalSigma", 3.2};
        m_resolutionTDC=10 * dd4hep::picosecond;//{this, "resolutionTDC", 10 * ps};

        // zero suppression values
        m_thresholdFactor=5.0;//{this, "thresholdFactor", 0.0};
        m_thresholdValue=2.0;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=0.03;//{this, "samplingFraction", 1.0};

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="EcalEndcapPInsertHits";  // from ATHENA's reconstruction.py
        m_layerField="";              // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="";             // from ATHENA's reconstruction.py

        m_localDetElement="";         // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={};          // from ATHENA's reconstruction.py (i.e. not defined there)

//        app->SetDefaultParameter("FEMC:tag",              m_input_tag);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:readout",          m_readout);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("FEMC:EcalEndcapPInsertRecHits:localDetFields",   u_localDetFields);
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
