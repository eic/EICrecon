// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

#pragma once

#include <algorithms/calorimetry/LogWeightReco.h>
#include <services/geometry/dd4hep/DD4hep_service.h>
#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>


namespace eicrecon {

class LogWeightReco_factoryT :
    public JChainMultifactoryT<LogWeightRecoConfig>,
    public SpdlogMixin {

  public:

    explicit LogWeightReco_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        LogWeightRecoConfig cfg)
    : JChainMultifactoryT<LogWeightRecoConfig>(std::move(tag), input_tags, output_tags, cfg) {

      DeclarePodioOutput<edm4eic::Cluster>(GetOutputTags()[0]);

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

        app->SetDefaultParameter(param_prefix + ":sampling_fraction",           cfg.sampling_fraction);
	app->SetDefaultParameter(param_prefix + ":E0",                          cfg.E0);
	app->SetDefaultParameter(param_prefix + ":w0_a",                        cfg.w0_a);
        app->SetDefaultParameter(param_prefix + ":w0_b",                        cfg.w0_b);
        app->SetDefaultParameter(param_prefix + ":w0_c",                        cfg.w0_c);
	

        m_algo.applyConfig(cfg);
        m_algo.init(geoSvc->detector(), logger());
    }

    void Process(const std::shared_ptr<const JEvent> &event) override {
        auto hits = static_cast<const edm4eic::CalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        try {
            auto clusters = m_algo.process(*hits);
            SetCollection<edm4eic::Cluster>(GetOutputTags()[0], std::move(clusters));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }

    private:
      LogWeightReco m_algo;

};

} // eicrecon
