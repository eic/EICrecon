// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/ImagingTopoCluster.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

  class ImagingTopoCluster_factoryT :
    public JChainMultifactoryT<ImagingTopoClusterConfig>,
    public SpdlogMixin {

  public:
    using SpdlogMixin::logger;

    explicit ImagingTopoCluster_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        ImagingTopoClusterConfig cfg)
    : JChainMultifactoryT<ImagingTopoClusterConfig>(std::move(tag), input_tags, output_tags, cfg) {

      DeclarePodioOutput<edm4eic::ProtoCluster>(GetOutputTags()[0]);

    }

    //------------------------------------------
    // Init
    void Init() override{

        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string param_prefix = GetPluginName() + ":" + GetTag();

        // SpdlogMixin logger initialization, sets m_log
        SpdlogMixin::InitLogger(app, GetPrefix(), "info");

        // Algorithm configuration
        auto cfg = GetDefaultConfig();

        app->SetDefaultParameter(param_prefix + ":localDistXY", cfg.localDistXY);
        app->SetDefaultParameter(param_prefix + ":layerDistEtaPhi", cfg.layerDistEtaPhi);
        app->SetDefaultParameter(param_prefix + ":neighbourLayersRange", cfg.neighbourLayersRange);
        app->SetDefaultParameter(param_prefix + ":sectorDist", cfg.sectorDist);
        app->SetDefaultParameter(param_prefix + ":minClusterHitEdep", cfg.minClusterHitEdep);
        app->SetDefaultParameter(param_prefix + ":minClusterCenterEdep", cfg.minClusterCenterEdep);
        app->SetDefaultParameter(param_prefix + ":minClusterEdep", cfg.minClusterEdep);
        app->SetDefaultParameter(param_prefix + ":minClusterNhits", cfg.minClusterNhits);

        m_algo.applyConfig(cfg);
        m_algo.init(logger());
    }

    void Process(const std::shared_ptr<const JEvent> &event) override {
        // Get input collection
        auto* hits = static_cast<const edm4eic::CalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        // Call Process for generic algorithm
        auto proto = m_algo.process(*hits);

        // Hand algorithm objects over to JANA
        SetCollection<edm4eic::ProtoCluster>(GetOutputTags()[0], std::move(proto));
    }

    private:
      eicrecon::ImagingTopoCluster m_algo;

  };

} // namespace eicrecon
