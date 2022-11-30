
#ifndef CalorimeterHit_factory_HcalEndcapPInsertRecHits_h_
#define CalorimeterHit_factory_HcalEndcapPInsertRecHits_h_

#include <JANA/JFactoryT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_HcalEndcapPInsertRecHits : public JFactoryT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

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
        m_thresholdFactor=4.0;//{this, "thresholdFactor", 0.0};
        m_thresholdValue=0.0;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=1.0;//{this, "samplingFraction", 1.0};

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="HcalEndcapPInsertHits";  // from ATHENA's reconstruction.py
        m_layerField="";              // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="";             // from ATHENA's reconstruction.py

        m_localDetElement="";         // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={};          // from ATHENA's reconstruction.py (i.e. not defined there)

//        app->SetDefaultParameter("HCAL:tag",              m_input_tag);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:readout",          m_readout);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("HCAL:HcalEndcapPInsertRecHits:localDetFields",   u_localDetFields);
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

#endif // CalorimeterHit_factory_HcalEndcapPInsertRecHits_h_
