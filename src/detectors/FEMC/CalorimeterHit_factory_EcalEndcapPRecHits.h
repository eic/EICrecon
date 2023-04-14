
#pragma once

#include <services/io/podio/JFactoryPodioT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_EcalEndcapPRecHits : public eicrecon::JFactoryPodioT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_EcalEndcapPRecHits(){
        SetTag("EcalEndcapPRecHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_input_tag = "EcalEndcapPRawHits";

        // digitization settings, must be consistent with digi class
        m_capADC=65536;//2^16 old: 16384
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
        m_readout="EcalEndcapPHits";  // from ATHENA's reconstruction.py
        m_layerField="";              // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="";             // from ATHENA's reconstruction.py

        m_localDetElement="";         // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={};          // from ATHENA's reconstruction.py (i.e. not defined there)

//        app->SetDefaultParameter("FEMC:tag",              m_input_tag);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:readout",          m_readout);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("FEMC:EcalEndcapPRecHits:localDetFields",   u_localDetFields);
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
