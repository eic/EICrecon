// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterHitsMerger.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

class CalorimeterHitsMerger_factoryT :
    public JChainMultifactoryT<CalorimeterHitsMergerConfig>,
    public SpdlogMixin {

  public:

    explicit CalorimeterHitsMerger_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        CalorimeterHitsMergerConfig cfg)
    : JChainMultifactoryT<CalorimeterHitsMergerConfig>(std::move(tag), input_tags, output_tags, cfg) {

      DeclarePodioOutput<edm4eic::CalorimeterHit>(GetOutputTags()[0]);

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

        app->SetDefaultParameter(param_prefix + ":fields", cfg.fields);
        app->SetDefaultParameter(param_prefix + ":refs",   cfg.refs);

        m_algo.applyConfig(cfg);
        m_algo.init(geoSvc->detector(), geoSvc->converter(), logger());
    }

    void Process(const std::shared_ptr<const JEvent> &event) override {
        auto raw_hits = event->GetCollection<edm4eic::CalorimeterHit>(GetInputTags()[0]);

        try {
            auto rec_hits = m_algo.process(*raw_hits);
            SetCollection<edm4eic::CalorimeterHit>(GetOutputTags()[0], std::move(rec_hits));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }

    private:
      CalorimeterHitsMerger m_algo;

};

} // eicrecon
