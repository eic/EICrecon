// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "algorithms/calorimetry/CalorimeterHitDigi.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

  class CalorimeterHitDigi_factoryT :
    public JChainMultifactoryT<CalorimeterHitDigiConfig>,
    public SpdlogMixin {

  public:

    explicit CalorimeterHitDigi_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        CalorimeterHitDigiConfig cfg)
    : JChainMultifactoryT<CalorimeterHitDigiConfig>(std::move(tag), input_tags, output_tags, cfg) {

      DeclarePodioOutput<edm4hep::RawCalorimeterHit>(GetOutputTags()[0]);

    }

    void Init() override {

        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string plugin_name  = GetPluginName();
        std::string param_prefix = plugin_name + ":" + GetTag();

        // Use DD4hep_service to get dd4hep::Detector
        auto geoSvc = app->template GetService<DD4hep_service>();

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(app, GetPrefix(), "info");

        // Algorithm configuration
        auto cfg = GetDefaultConfig();

        app->SetDefaultParameter(param_prefix + ":energyResolutions",cfg.eRes);
        app->SetDefaultParameter(param_prefix + ":timeResolution",   cfg.tRes);
        app->SetDefaultParameter(param_prefix + ":capacityADC",      cfg.capADC);
        app->SetDefaultParameter(param_prefix + ":dynamicRangeADC",  cfg.dyRangeADC);
        app->SetDefaultParameter(param_prefix + ":pedestalMean",     cfg.pedMeanADC);
        app->SetDefaultParameter(param_prefix + ":pedestalSigma",    cfg.pedSigmaADC);
        app->SetDefaultParameter(param_prefix + ":resolutionTDC",    cfg.resolutionTDC);
        app->SetDefaultParameter(param_prefix + ":scaleResponse",    cfg.corrMeanScale);
        app->SetDefaultParameter(param_prefix + ":signalSumFields",  cfg.fields);
        app->SetDefaultParameter(param_prefix + ":readoutClass",     cfg.readout);

        m_algo.applyConfig(cfg);
        m_algo.init(geoSvc->detector(), logger());
    }

    void Process(const std::shared_ptr<const JEvent> &event) override {
        auto hits = static_cast<const edm4hep::SimCalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        try {
            auto raw_hits = m_algo.process(*hits);
            SetCollection<edm4hep::RawCalorimeterHit>(GetOutputTags()[0], std::move(raw_hits));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }

    private:
      CalorimeterHitDigi m_algo;

};

} // eicrecon
