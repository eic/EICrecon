// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/ImagingPixelReco.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

class ImagingPixelReco_factoryT :
    public JChainMultifactoryT<ImagingPixelRecoConfig>,
    public SpdlogMixin {

  public:
    using SpdlogMixin::logger;

    explicit ImagingPixelReco_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        ImagingPixelRecoConfig cfg)
    : JChainMultifactoryT<ImagingPixelRecoConfig>(std::move(tag), input_tags, output_tags, cfg) {

         DeclarePodioOutput<edm4eic::CalorimeterHit>(GetOutputTags()[0]);

    }

    //------------------------------------------
    // Init
    void Init() override {

        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string plugin_name  = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
        std::string param_prefix = plugin_name + ":" + GetTag();

        // Use JDD4hep_service to get dd4hep::Detector
        auto geoSvc = app->template GetService<JDD4hep_service>();

        // SpdlogMixin logger initialization, sets m_log
        SpdlogMixin::InitLogger(app, GetPrefix(), "info");

        // Algorithm configuration
        auto cfg = GetDefaultConfig();

        app->SetDefaultParameter(param_prefix + ":layerField",       cfg.layerField);
        app->SetDefaultParameter(param_prefix + ":sectorField",      cfg.sectorField);
        app->SetDefaultParameter(param_prefix + ":capacityADC",      cfg.capADC);
        app->SetDefaultParameter(param_prefix + ":pedestalMean",     cfg.pedMeanADC);
        app->SetDefaultParameter(param_prefix + ":dynamicRangeADC",  cfg.dyRangeADC);
        app->SetDefaultParameter(param_prefix + ":pedSigmaADC",      cfg.pedSigmaADC);
        app->SetDefaultParameter(param_prefix + ":thresholdFactor",  cfg.thresholdFactor);
        app->SetDefaultParameter(param_prefix + ":samplingFraction", cfg.sampFrac);

        m_algo.applyConfig(cfg);
        m_algo.init(geoSvc->detector(), logger());
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override {

        auto raw_hits = static_cast<const edm4hep::RawCalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        try {
            auto rec_hits = m_algo.process(*raw_hits);
            SetCollection<edm4eic::CalorimeterHit>(GetOutputTags()[0], std::move(rec_hits));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }

    private:
      eicrecon::ImagingPixelReco m_algo;

};

} // eicrecon
