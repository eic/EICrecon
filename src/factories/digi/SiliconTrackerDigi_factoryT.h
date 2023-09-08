// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/digi/SiliconTrackerDigi.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

  class SiliconTrackerDigi_factoryT :
    public JChainMultifactoryT<SiliconTrackerDigiConfig>,
    public SpdlogMixin {

  public:

    explicit SiliconTrackerDigi_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        SiliconTrackerDigiConfig cfg)
    : JChainMultifactoryT<SiliconTrackerDigiConfig>(std::move(tag), input_tags, output_tags, cfg) {

      DeclarePodioOutput<edm4eic::RawTrackerHit>(GetOutputTags()[0]);

    }

    void Init() override {

        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string plugin_name  = GetPluginName();
        std::string param_prefix = plugin_name + ":" + GetTag();

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(app, GetPrefix(), "info");

        // Algorithm configuration
        auto cfg = GetDefaultConfig();

        app->SetDefaultParameter(param_prefix + ":threshold",      cfg.threshold);
        app->SetDefaultParameter(param_prefix + ":timeResolution", cfg.timeResolution);

        m_algo.applyConfig(cfg);
        m_algo.init(logger());
    }

    void Process(const std::shared_ptr<const JEvent> &event) override {
        auto hits = event->GetCollection<edm4hep::SimTrackerHit>(GetInputTags()[0]);

        try {
            auto raw_hits = m_algo.process(*hits);
            SetCollection<edm4eic::RawTrackerHit>(GetOutputTags()[0], std::move(raw_hits));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }

    private:
      SiliconTrackerDigi m_algo;

};

} // eicrecon
