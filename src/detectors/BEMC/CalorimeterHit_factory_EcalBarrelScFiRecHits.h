
#pragma once

#include "extensions/jana/JChainFactoryT.h"

#include "algorithms/calorimetry/CalorimeterHitReco.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"

class CalorimeterHit_factory_EcalBarrelScFiRecHits : public JChainFactoryT<edm4eic::CalorimeterHit>, CalorimeterHitReco {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_EcalBarrelScFiRecHits(std::vector<std::string> default_input_tags)
    : JChainFactoryT<edm4eic::CalorimeterHit>(std::move(default_input_tags)) {
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        InitDataTags(GetPluginName() + ":" + GetTag());

        auto app = GetApplication();

        // digitization settings, must be consistent with digi class
        m_capADC=16384;//{this, "capacityADC", 8096};
        m_dyRangeADC=750. * dd4hep::MeV;//{this, "dynamicRangeADC", 100. * dd4hep::MeV};
        m_pedMeanADC=20;//{this, "pedestalMean", 400};
        m_pedSigmaADC=0.3;//{this, "pedestalSigma", 3.2};
        m_resolutionTDC=10 * dd4hep::picosecond;//{this, "resolutionTDC", 10 * ps};

        // zero suppression values
        m_thresholdFactor=36.0488;// from ATHENA's reconstruction.py
        m_thresholdValue=0.0;//{this, "thresholdValue", 0.0};

        // energy correction with sampling fraction
        m_sampFrac=0.10200085;// from ${DETECTOR_PATH}/calibrations/emcal_barrel_calibration.json

        // geometry service to get ids, ignored if no names provided
        m_geoSvcName="geoServiceName";
        m_readout="EcalBarrelScFiHits";  // from ATHENA's reconstruction.py
        m_layerField="layer";            // from ATHENA's reconstruction.py (i.e. not defined there)
        m_sectorField="module";          // from ATHENA's reconstruction.py

        m_localDetElement="";            // from ATHENA's reconstruction.py (i.e. not defined there)
        u_localDetFields={"system"};     // from ATHENA's reconstruction.py (i.e. not defined there)

        // here we want to use grid center position (XY) but keeps the z information from fiber-segment
        // TODO: a more realistic way to get z is to reconstruct it from timing
        m_maskPos = "xy";
        u_maskPosFields = {"fiber", "z"};


        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:readout",          m_readout );
        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:layerField",       m_layerField );
        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:sectorField",      m_sectorField );
        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:thresholdValue",   m_thresholdValue);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:samplingFraction", m_sampFrac);
        app->SetDefaultParameter("BEMC:EcalBarrelScFiRecHits:maskPosition",     m_maskPos);
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
        // Get input collection
        auto rawhits_coll = static_cast<const edm4hep::RawCalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        // Call Process for generic algorithm
        auto recohits_coll = AlgorithmProcess(*rawhits_coll);

        // Hand algorithm objects over to JANA
        SetCollection(std::move(recohits_coll));
    }

};
