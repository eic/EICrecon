// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "algorithms/calorimetry/ImagingClusterReco.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

class ImagingClusterReco_factoryT :
    public JChainMultifactoryT<ImagingClusterRecoConfig>,
    public SpdlogMixin {

  public:
    using SpdlogMixin::logger;

    explicit ImagingClusterReco_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        ImagingClusterRecoConfig cfg)
    : JChainMultifactoryT<ImagingClusterRecoConfig>(std::move(tag), input_tags, output_tags, cfg) {

      DeclarePodioOutput<edm4eic::Cluster>(GetOutputTags()[0]);
      DeclarePodioOutput<edm4eic::MCRecoClusterParticleAssociation>(GetOutputTags()[1]);
      DeclarePodioOutput<edm4eic::Cluster>(GetOutputTags()[2]);

    }

    //------------------------------------------
    // Init
    void Init() override {

        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string plugin_name  = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
        std::string param_prefix = plugin_name + ":" + GetTag();

        // SpdlogMixin logger initialization, sets m_log
        SpdlogMixin::InitLogger(app, GetPrefix(), "info");

        // Algorithm configuration
        auto cfg = GetDefaultConfig();

        app->SetDefaultParameter(param_prefix + ":trackStopLayer", cfg.trackStopLayer);

        m_algo.applyConfig(cfg);
        m_algo.init(logger());
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override {

        // TODO: NWB: We are using GetCollectionBase because GetCollection is temporarily out of commission due to JFactoryPodioTFixed
        auto proto = static_cast<const edm4eic::ProtoClusterCollection*>(event->GetCollectionBase(GetInputTags()[0]));
        auto mchits = static_cast<const edm4hep::SimCalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[1]));

        try {
            auto clusters = m_algo.process(*proto, *mchits);
            SetCollection<edm4eic::Cluster>(GetOutputTags()[0], std::move(std::get<0>(clusters)));
            SetCollection<edm4eic::MCRecoClusterParticleAssociation>(GetOutputTags()[1], std::move(std::get<1>(clusters)));
            SetCollection<edm4eic::Cluster>(GetOutputTags()[2], std::move(std::get<2>(clusters)));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }

    private:
      eicrecon::ImagingClusterReco m_algo;

};

} // eicrecon
