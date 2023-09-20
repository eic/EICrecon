// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterIslandCluster.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

class CalorimeterIslandCluster_factoryT :
    public JChainMultifactoryT<CalorimeterIslandClusterConfig>,
    public SpdlogMixin {

  public:

    explicit CalorimeterIslandCluster_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        CalorimeterIslandClusterConfig cfg)
    : JChainMultifactoryT<CalorimeterIslandClusterConfig>(std::move(tag), input_tags, output_tags, cfg) {

      DeclarePodioOutput<edm4eic::ProtoCluster>(GetOutputTags()[0]);

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

        // Remove spaces from adjacency matrix
        cfg.adjacencyMatrix.erase(
          std::remove_if(cfg.adjacencyMatrix.begin(), cfg.adjacencyMatrix.end(), ::isspace),
          cfg.adjacencyMatrix.end());

        app->SetDefaultParameter(param_prefix + ":sectorDist", cfg.sectorDist);
        app->SetDefaultParameter(param_prefix + ":localDistXY", cfg.localDistXY);
        app->SetDefaultParameter(param_prefix + ":localDistXZ", cfg.localDistXZ);
        app->SetDefaultParameter(param_prefix + ":localDistYZ", cfg.localDistYZ);
        app->SetDefaultParameter(param_prefix + ":globalDistRPhi", cfg.globalDistRPhi);
        app->SetDefaultParameter(param_prefix + ":globalDistEtaPhi", cfg.globalDistEtaPhi);
        app->SetDefaultParameter(param_prefix + ":dimScaledLocalDistXY", cfg.dimScaledLocalDistXY);
        app->SetDefaultParameter(param_prefix + ":adjacencyMatrix", cfg.adjacencyMatrix);
        app->SetDefaultParameter(param_prefix + ":readoutClass", cfg.readout);
        app->SetDefaultParameter(param_prefix + ":splitCluster", cfg.splitCluster);
        app->SetDefaultParameter(param_prefix + ":minClusterHitEdep", cfg.minClusterHitEdep);
        app->SetDefaultParameter(param_prefix + ":minClusterCenterEdep", cfg.minClusterCenterEdep);
        app->SetDefaultParameter(param_prefix + ":transverseEnergyProfileMetric", cfg.transverseEnergyProfileMetric);
        app->SetDefaultParameter(param_prefix + ":transverseEnergyProfileScale", cfg.transverseEnergyProfileScale);

        m_algo.applyConfig(cfg);
        m_algo.init(geoSvc->detector(), logger());
    }

    void Process(const std::shared_ptr<const JEvent> &event) override {
        auto hits = static_cast<const edm4eic::CalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        try {
            auto proto_clusters = m_algo.process(*hits);
            SetCollection<edm4eic::ProtoCluster>(GetOutputTags()[0], std::move(proto_clusters));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }

    private:
      CalorimeterIslandCluster m_algo;

};

} // eicrecon
