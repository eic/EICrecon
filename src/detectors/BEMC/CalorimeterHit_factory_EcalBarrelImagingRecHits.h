
#ifndef CalorimeterHit_factory_EcalBarrelImagingRecHits_h_
#define CalorimeterHit_factory_EcalBarrelImagingRecHits_h_

#include <JANA/JFactoryT.h>

#include <algorithms/calorimetry/ImagingPixelReco.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_EcalBarrelImagingRecHits : public JFactoryT<edm4eic::CalorimeterHit>, ImagingPixelReco {

public:

    std::string m_input_tag  = "EcalBarrelImagingRawHits";


    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_EcalBarrelImagingRecHits(){
        SetTag("EcalBarrelImagingRecHits");
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();

        m_readout = "EcalBarrelHits";
        m_layerField = "layer"; // {this, "layerField", "layer"};
        m_sectorField = "module"; // {this, "sectorField", "sector"};
        // length unit (from dd4hep geometry service)
        m_lUnit = dd4hep::mm; // {this, "lengthUnit", dd4hep::mm};
        // digitization parameters
        m_capADC=8096; // {this, "capacityADC", 8096};
        m_pedMeanADC=400; // {this, "pedestalMean", 400};
        m_dyRangeADC=100 * MeV; // {this, "dynamicRangeADC", 100 * MeV};
        m_pedSigmaADC=3.2; // {this, "pedestalSigma", 3.2};
        m_thresholdADC=3.0; // {this, "thresholdFactor", 3.0};
        // Calibration!
        m_sampFrac=0.10262666247845109;// from ${DETECTOR_PATH}/calibrations/emcal_barrel_calibration.json

        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:input_tag",        m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:layerField",       m_layerField);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:sectorField",      m_sectorField);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:capacityADC",      m_capADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:pedestalSigmaADC", m_pedSigmaADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:pedSigmaADC",      m_pedSigmaADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:thresholdADC",     m_thresholdADC);
        app->SetDefaultParameter("BEMC:EcalBarrelImagingRecHits:samplingFraction", m_sampFrac);
        m_geoSvc = app->template GetService<JDD4hep_service>(); // TODO: implement named geometry service?

        std::string tag=this->GetTag();
        m_log = app->GetService<Log_service>()->logger(tag);

        // Get log level from user parameter or default
        std::string log_level_str = "info";
        auto pm = app->GetJParameterManager();
        pm->SetDefaultParameter(tag + ":LogLevel", log_level_str, "verbosity: trace, debug, info, warn, err, critical, off");
        m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

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

#endif // CalorimeterHit_factory_EcalBarrelImagingRecHits_h_
