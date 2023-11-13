// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "algorithms/calorimetry/CalorimeterClusterRecoCoG.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

class CalorimeterClusterRecoCoG_factoryT :
    public JChainMultifactoryT<CalorimeterClusterRecoCoGConfig>,
    public SpdlogMixin {

  public:

    explicit CalorimeterClusterRecoCoG_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        CalorimeterClusterRecoCoGConfig cfg)
    : JChainMultifactoryT<CalorimeterClusterRecoCoGConfig>(std::move(tag), input_tags, output_tags, cfg),
      m_algo(tag) {

      DeclarePodioOutput<edm4eic::Cluster>(GetOutputTags()[0]);
      DeclarePodioOutput<edm4eic::MCRecoClusterParticleAssociation>(GetOutputTags()[1]);

    }

    //------------------------------------------
    // Init
    void Init() override {

        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string plugin_name  = GetPluginName();
        std::string param_prefix = plugin_name + ":" + GetTag();

        // Use DD4hep_service to get dd4hep::Detector
        auto geoSvc = app->template GetService<DD4hep_service>();
        auto detector = geoSvc->detector();

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(app, GetPrefix(), "info");

        // Algorithm configuration
        auto cfg = GetDefaultConfig();

        app->SetDefaultParameter(param_prefix + ":samplingFraction", cfg.sampFrac);
        app->SetDefaultParameter(param_prefix + ":logWeightBase", cfg.logWeightBase);
        app->SetDefaultParameter(param_prefix + ":energyWeight", cfg.energyWeight);
        app->SetDefaultParameter(param_prefix + ":enableEtaBounds", cfg.enableEtaBounds);

        m_algo.applyConfig(cfg);
        m_algo.init(detector, logger());
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override {

        // TODO: NWB: We are using GetCollectionBase because GetCollection is temporarily out of commission due to JFactoryPodioTFixed
        auto proto = static_cast<const edm4eic::ProtoClusterCollection*>(event->GetCollectionBase(GetInputTags()[0]));
        auto mchits = static_cast<const edm4hep::SimCalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[1]));

        try {
            auto clusters = std::make_unique<edm4eic::ClusterCollection>();
            auto assocs = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();

            decltype(m_algo)::Input input{proto, mchits};
            decltype(m_algo)::Output output{clusters.get(), assocs.get()};

            m_algo.process(input, output);

            SetCollection<edm4eic::Cluster>(GetOutputTags()[0], std::move(clusters));
            SetCollection<edm4eic::MCRecoClusterParticleAssociation>(GetOutputTags()[1], std::move(assocs));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }

    private:
      eicrecon::CalorimeterClusterRecoCoG m_algo;

};

} // eicrecon
