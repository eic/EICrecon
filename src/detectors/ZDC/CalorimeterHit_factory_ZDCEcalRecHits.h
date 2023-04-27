
#pragma once

#include <services/io/podio/JFactoryPodioT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_ZDCEcalRecHits : public eicrecon::JFactoryPodioT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_ZDCEcalRecHits(){
        SetTag("ZDCEcalRecHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_input_tag = "ZDCEcalRawHits";

        // digitization settings, must be consistent with digi class
        m_capADC=8096;//{this, "capacityADC", 8096};
        m_dyRangeADC=100. * dd4hep::MeV;//{this, "dynamicRangeADC", 100. * dd4hep::MeV};
        m_pedMeanADC=400;//{this, "pedestalMean", 400};
        m_pedSigmaADC=3.2;//{this, "pedestalSigma", 3.2};
        m_resolutionTDC=10 * dd4hep::picosecond;//{this, "resolutionTDC", 10 * ps};

        // zero suppression values
        m_thresholdFactor=4.0;//{this, "thresholdFactor", 0.0};
        m_thresholdValue=0.0;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=1.0;//{this, "samplingFraction", 1.0};

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="ZDCEcalHits";  // from ATHENA's reconstruction.py
        m_layerField="";              // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="";             // from ATHENA's reconstruction.py

        m_localDetElement="";         // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={};          // from ATHENA's reconstruction.py (i.e. not defined there)

//        app->SetDefaultParameter("ZDC:tag",              m_input_tag);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:readout",          m_readout);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("ZDC:ZDCEcalRecHits:localDetFields",   u_localDetFields);
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
