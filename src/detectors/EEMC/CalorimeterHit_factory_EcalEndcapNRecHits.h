
#ifndef CalorimeterHit_factory_EcalEndcapNRecHits_h_
#define CalorimeterHit_factory_EcalEndcapNRecHits_h_

#include <JANA/JFactoryT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>

class CalorimeterHit_factory_EcalEndcapNRecHits : public JFactoryT<eicd::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_EcalEndcapNRecHits(){
        SetTag("EcalEndcapNRecHits");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_input_tag = "EcalEndcapNRawHits";

        // digitization settings, must be consistent with digi class
        m_capADC=8096;//{this, "capacityADC", 8096};
        m_dyRangeADC=100. * MeV;//{this, "dynamicRangeADC", 100. * MeV};
        m_pedMeanADC=400;//{this, "pedestalMean", 400};
        m_pedSigmaADC=3.2;//{this, "pedestalSigma", 3.2};
        m_resolutionTDC=10 * dd4hep::picosecond;//{this, "resolutionTDC", 10 * ps};

        // zero suppression values
        m_thresholdFactor=4.0;//{this, "thresholdFactor", 0.0};
        m_thresholdValue=0.0;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=0.998;//{this, "samplingFraction", 1.0};

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="EcalEndcapNHits";  // from ATHENA's reconstruction.py
        m_layerField="";              // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="sector";       // from ATHENA's reconstruction.py

        m_localDetElement="";         // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={};          // from ATHENA's reconstruction.py (i.e. not defined there)

//        app->SetDefaultParameter("EEMC:tag",              m_input_tag);
        app->SetDefaultParameter("EEMC:capacityADC",      m_capADC);
        app->SetDefaultParameter("EEMC:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("EEMC:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("EEMC:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("EEMC:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("EEMC:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("EEMC:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("EEMC:samplingFraction", m_sampFrac);
        m_geoSvc = app->template GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        AlgorithmInit();
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

#endif // CalorimeterHit_factory_EcalEndcapNRecHits_h_
