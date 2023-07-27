// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "algorithms/calorimetry/ImagingTopoCluster.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

  class ProtoCluster_factory_EcalBarrelImagingProtoClusters :
    public JChainMultifactoryT<ImagingTopoClusterConfig>,
    public SpdlogMixin<ProtoCluster_factory_EcalBarrelImagingProtoClusters> {

  public:
    using SpdlogMixin<ProtoCluster_factory_EcalBarrelImagingProtoClusters>::logger;

    explicit ProtoCluster_factory_EcalBarrelImagingProtoClusters(
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
        std::string plugin_name  = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
        std::string param_prefix = plugin_name + ":" + GetTag();

        // SpdlogMixin logger initialization, sets m_log
        SpdlogMixin<ProtoCluster_factory_EcalBarrelImagingProtoClusters>::InitLogger(JChainMultifactoryT<ImagingTopoClusterConfig>::GetPrefix(), "info");

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
