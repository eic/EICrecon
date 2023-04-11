
#ifndef CalorimeterHit_factory_LFHCALRecHits_h_
#define CalorimeterHit_factory_LFHCALRecHits_h_

#include <JANA/JFactoryT.h>

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_LFHCALRecHits : public JFactoryT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_LFHCALRecHits(){
        SetTag("LFHCALRecHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_input_tag = "LFHCALRawHits";

        // digitization settings, must be consistent with digi class
        m_capADC=65536;//2^16
        m_dyRangeADC=1 * dd4hep::GeV;//{this, "dynamicRangeADC", 100. * dd4hep::MeV};
        m_pedMeanADC=20;//{this, "pedestalMean", 400};
        m_pedSigmaADC=0.8;//{this, "pedestalSigma", 3.2};
        m_resolutionTDC=10 * dd4hep::picosecond;//{this, "resolutionTDC", 10 * ps};

        // zero suppression values
        m_thresholdFactor=1.0;//{this, "thresholdFactor", 0.0};
        m_thresholdValue=3.0;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=0.033;//{this, "samplingFraction", 1.0};
//         m_sampFracLayer[0]=0.019;
        m_sampFracLayer[0]=0.037;
        for (int i = 1; i < 13; i++) m_sampFracLayer[i]=0.037;
        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="LFHCALHits";
        m_layerField="";
        m_sectorField="";

        m_localDetElement="";
        u_localDetFields={};

//        app->SetDefaultParameter("FHCAL:tag",              m_input_tag);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:geoServiceName",   m_geoSvcName);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:readout",          m_readout);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:localDetElement",  m_localDetElement);
        app->SetDefaultParameter("FHCAL:LFHCALRecHits:localDetFields",   u_localDetFields);
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

#endif // CalorimeterHit_factory_LFHCALRecHits_h_
