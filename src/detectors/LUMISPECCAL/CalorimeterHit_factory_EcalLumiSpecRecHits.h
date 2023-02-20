
#pragma once


#include <JANA/JFactoryT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_EcalLumiSpecRecHits : public JFactoryT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_EcalLumiSpecRecHits(){
        SetTag("EcalLumiSpecRecHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_input_tag = "EcalLumiSpecRawHits";

        // digitization settings, must be consistent with digi class
        m_capADC=16384;//{this, "capacityADC", 8096};
        m_dyRangeADC=20. * dd4hep::GeV;//{this, "dynamicRangeADC", 100. * dd4hep::MeV};
        m_pedMeanADC=100;//{this, "pedestalMean", 400};
        m_pedSigmaADC=1;//{this, "pedestalSigma", 3.2};
        m_resolutionTDC=10 * dd4hep::picosecond;//{this, "resolutionTDC", 10 * ps};

        // zero suppression values
        m_thresholdFactor=4.0;//{this, "thresholdFactor", 0.0};
        m_thresholdValue=3.0;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=0.998;//{this, "samplingFraction", 1.0};

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="LumiSpecCALHits";  // from ATHENA's reconstruction.py
        m_layerField="";              // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="sector";       // from ATHENA's reconstruction.py

        m_localDetElement="";         // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={};          // from ATHENA's reconstruction.py (i.e. not defined there)

//        app->SetDefaultParameter("EEMC:tag",              m_input_tag);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:readout",          m_readout);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("LUMISPECCAL:EcalLumiSpecRecHits:localDetFields",   u_localDetFields);
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

