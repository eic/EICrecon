// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterHitsMerger.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

// variadic template parameter T unused
template<template<typename> typename... T>
class CalorimeterHitsMerger_factoryT :
    public JChainMultifactoryT<CalorimeterHitsMergerConfig>,
    public SpdlogMixin<CalorimeterHitsMerger_factoryT<T...>>,
    public T<CalorimeterHitsMerger_factoryT<T...>>... {

  public:
    using SpdlogMixin<CalorimeterHitsMerger_factoryT>::logger;

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
        std::string plugin_name  = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
        std::string param_prefix = plugin_name + ":" + GetTag();

        // Use JDD4hep_service to get dd4hep::Detector
        auto geoSvc = app->template GetService<JDD4hep_service>();

        // SpdlogMixin logger initialization, sets m_log
        SpdlogMixin<CalorimeterHitsMerger_factoryT>::InitLogger(JChainMultifactoryT<CalorimeterHitsMergerConfig>::GetPrefix(), "info");

        // Algorithm configuration
        auto cfg = GetDefaultConfig();

        app->SetDefaultParameter(param_prefix + ":fields", cfg.fields);
        app->SetDefaultParameter(param_prefix + ":refs",   cfg.refs);

        m_algo.applyConfig(cfg);
        m_algo.init(geoSvc->detector(), logger());
    }

    void Process(const std::shared_ptr<const JEvent> &event) override {
        auto raw_hits = static_cast<const edm4eic::CalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

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
