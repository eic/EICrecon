
#ifndef CalorimeterHit_factory_HcalEndcapPRecHits_h_
#define CalorimeterHit_factory_HcalEndcapPRecHits_h_

#include <JANA/JFactoryT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_HcalEndcapPRecHits : public JFactoryT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

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
        m_capADC=4096;//{this, "capacityADC", 8096};
        m_dyRangeADC=200. * dd4hep::MeV;//{this, "dynamicRangeADC", 100. * dd4hep::MeV};
        m_pedMeanADC=200;//{this, "pedestalMean", 400};
        m_pedSigmaADC=3.2;//{this, "pedestalSigma", 3.2};
        m_resolutionTDC=10 * dd4hep::picosecond;//{this, "resolutionTDC", 10 * ps};

        // zero suppression values
        m_thresholdFactor=4.0;//{this, "thresholdFactor", 0.0};
        m_thresholdValue=0.0;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=0.033;//{this, "samplingFraction", 1.0};

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="HcalEndcapPHits";  // from ATHENA's reconstruction.py
        m_layerField="";              // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="";             // from ATHENA's reconstruction.py

        m_localDetElement="";         // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={};          // from ATHENA's reconstruction.py (i.e. not defined there)

//        app->SetDefaultParameter("HCAL:tag",              m_input_tag);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:readout",          m_readout);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("HCAL:HcalEndcapPRecHits:localDetFields",   u_localDetFields);
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

#endif // CalorimeterHit_factory_HcalEndcapPRecHits_h_
