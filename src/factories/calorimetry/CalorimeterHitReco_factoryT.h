// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>


namespace eicrecon {

// variadic template parameter T unused
template<template<typename> typename... T>
class CalorimeterHitReco_factoryT :
    public JChainMultifactoryT<CalorimeterHitRecoConfig>,
    public SpdlogMixin<CalorimeterHitReco_factoryT<T...>>,
    public T<CalorimeterHitReco_factoryT<T...>>... {

  public:
    using SpdlogMixin<CalorimeterHitReco_factoryT>::logger;

    explicit CalorimeterHitReco_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        CalorimeterHitRecoConfig cfg)
    : JChainMultifactoryT<CalorimeterHitRecoConfig>(std::move(tag), input_tags, output_tags, cfg) {

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
        SpdlogMixin<CalorimeterHitReco_factoryT>::InitLogger(JChainMultifactoryT<CalorimeterHitRecoConfig>::GetPrefix(), "info");

        // Algorithm configuration
        auto cfg = GetDefaultConfig();

        app->SetDefaultParameter(param_prefix + ":capacityADC",      cfg.capADC);
        app->SetDefaultParameter(param_prefix + ":dynamicRangeADC",  cfg.dyRangeADC);
        app->SetDefaultParameter(param_prefix + ":pedestalMean",     cfg.pedMeanADC);
        app->SetDefaultParameter(param_prefix + ":pedestalSigma",    cfg.pedSigmaADC);
        app->SetDefaultParameter(param_prefix + ":resolutionTDC",    cfg.resolutionTDC);
        app->SetDefaultParameter(param_prefix + ":thresholdFactor",  cfg.thresholdFactor);
        app->SetDefaultParameter(param_prefix + ":thresholdValue",   cfg.thresholdValue);
        app->SetDefaultParameter(param_prefix + ":samplingFraction", cfg.sampFrac);
        app->SetDefaultParameter(param_prefix + ":readout",          cfg.readout);
        app->SetDefaultParameter(param_prefix + ":layerField",       cfg.layerField);
        app->SetDefaultParameter(param_prefix + ":sectorField",      cfg.sectorField);
        app->SetDefaultParameter(param_prefix + ":localDetElement",  cfg.localDetElement);
        app->SetDefaultParameter(param_prefix + ":localDetFields",   cfg.localDetFields);

        m_algo.applyConfig(cfg);
        m_algo.init(geoSvc->detector(), logger());
    }

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
      CalorimeterHitReco m_algo;

};

} // eicrecon
