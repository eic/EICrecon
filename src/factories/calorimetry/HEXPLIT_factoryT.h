// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

#pragma once

#include <algorithms/calorimetry/HEXPLIT.h>
#include <services/geometry/dd4hep/DD4hep_service.h>
#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>


namespace eicrecon {

class HEXPLIT_factoryT :
    public JChainMultifactoryT<HEXPLITConfig>,
    public SpdlogMixin {

  public:

    explicit HEXPLIT_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        HEXPLITConfig cfg)
    : JChainMultifactoryT<HEXPLITConfig>(std::move(tag), input_tags, output_tags, cfg) {

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

        app->SetDefaultParameter(param_prefix + ":MIP",           cfg.MIP);
        app->SetDefaultParameter(param_prefix + ":Emin",          cfg.Emin);
        app->SetDefaultParameter(param_prefix + ":tmax",          cfg.tmax);
        app->SetDefaultParameter(param_prefix + ":side_length",   cfg.side_length);
        app->SetDefaultParameter(param_prefix + ":layer_spacing", cfg.layer_spacing);
        app->SetDefaultParameter(param_prefix + ":rot_x",         cfg.rot_x);
	app->SetDefaultParameter(param_prefix + ":rot_y",         cfg.rot_y);
	app->SetDefaultParameter(param_prefix + ":rot_z",         cfg.rot_z);
	app->SetDefaultParameter(param_prefix + ":trans_x",       cfg.trans_x);
        app->SetDefaultParameter(param_prefix + ":trans_y",       cfg.trans_y);
        app->SetDefaultParameter(param_prefix + ":trans_z",       cfg.trans_z);
	

        m_algo.applyConfig(cfg);
        m_algo.init(geoSvc->detector(), logger());
    }

    void Process(const std::shared_ptr<const JEvent> &event) override {
        auto hits = static_cast<const edm4eic::CalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        try {
            auto subcell_hits = m_algo.process(*hits);
            SetCollection<edm4eic::CalorimeterHit>(GetOutputTags()[0], std::move(subcell_hits));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }

    private:
      HEXPLIT m_algo;

};

} // eicrecon
