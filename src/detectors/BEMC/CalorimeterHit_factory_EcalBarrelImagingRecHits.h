
#pragma once

#include <services/io/podio/JFactoryPodioT.h>

#include <algorithms/calorimetry/ImagingPixelReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_EcalBarrelImagingRecHits : public eicrecon::JFactoryPodioT<edm4eic::CalorimeterHit>, ImagingPixelReco {

public:

    std::string m_input_tag  = "EcalBarrelImagingRawHits";


    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_EcalBarrelImagingRecHits(){
        SetTag("EcalBarrelImagingRecHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_readout = "EcalBarrelImagingHits";
        m_layerField = "layer"; // {this, "layerField", "layer"};
        m_sectorField = "module"; // {this, "sectorField", "sector"};
        // length unit (from dd4hep geometry service)
        m_lUnit = dd4hep::mm; // {this, "lengthUnit", dd4hep::mm};
        // digitization parameters
        m_capADC=8192; // {this, "capacityADC", 8096};
        m_pedMeanADC=100; // {this, "pedestalMean", 400};
        m_dyRangeADC=3 * dd4hep::MeV;   // {this, "dynamicRangeADC", 100 * MeV};
        m_pedSigmaADC=14; // {this, "pedestalSigma", 3.2};
        m_thresholdFactor=3.0; // {this, "thresholdFactor", 3.0};
        // Calibration!
        m_sampFrac=0.005;// from ${DETECTOR_PATH}/calibrations/emcal_barrel_calibration.json

        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:pedSigmaADC",      m_pedSigmaADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:thresholdFactor",  m_thresholdFactor);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:samplingFraction", m_sampFrac);
        m_geoSvc = app->template GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        initialize();
    }


    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override{
        // Prefill inputs
        m_inputHits = event->Get<edm4hep::RawCalorimeterHit>(m_input_tag);

        // Call Process for generic algorithm
        execute();

        // Hand owner of algorithm objects over to JANA
        Set(m_outputHits);
        m_outputHits.clear(); // not really needed, but better to not leave dangling pointers around
    }

};
