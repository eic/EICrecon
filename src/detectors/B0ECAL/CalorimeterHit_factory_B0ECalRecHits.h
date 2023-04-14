
#pragma once


#include <services/io/podio/JFactoryPodioT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_B0ECalRecHits : public eicrecon::JFactoryPodioT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_B0ECalRecHits(){
        SetTag("B0ECalRecHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_input_tag = "B0ECalRawHits";

        // digitization settings, must be consistent with digi class
        m_capADC=16384;//{this, "capacityADC", 8096};
        m_dyRangeADC=20. * dd4hep::GeV;//{this, "dynamicRangeADC", 100. * dd4hep::MeV};
        m_pedMeanADC=100;//{this, "pedestalMean", 400};
        m_pedSigmaADC=1;//{this, "pedestalSigma", 3.2};
        m_resolutionTDC=1e-11;//{this, "resolutionTDC", 10 * ps};

        // zero suppression values
        m_thresholdFactor=4.0;//{this, "thresholdFactor", 0.0};
        m_thresholdValue=3.0;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=0.998;//{this, "samplingFraction", 1.0};

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="";
        m_readout="B0ECalHits";  // from ATHENA's reconstruction.py
        m_layerField="";              // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="sector";       // from ATHENA's reconstruction.py

        m_localDetElement="";         // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={};          // from ATHENA's reconstruction.py (i.e. not defined there)

        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:readout",          m_readout);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("B0ECAL:B0ECalRecHits:localDetFields",   u_localDetFields);
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
