// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "algorithms/calorimetry/CalorimeterTruthClustering.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

class CalorimeterTruthClustering_factoryT :
    public JChainMultifactoryT<NoConfig>,
    public SpdlogMixin {

  public:

    explicit CalorimeterTruthClustering_factoryT(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags)
    : JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {

      DeclarePodioOutput<edm4eic::ProtoCluster>(GetOutputTags()[0]);

    }

    void Init() override {

        auto app = GetApplication();

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(app, GetPrefix(), "info");

        m_algo.init(logger());
    }

    void Process(const std::shared_ptr<const JEvent> &event) override {
        auto rc_hits = static_cast<const edm4eic::CalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));
        auto mc_hits = static_cast<const edm4hep::SimCalorimeterHitCollection*>(event->GetCollectionBase(GetInputTags()[1]));

        try {
            auto clusters = m_algo.process(*rc_hits, *mc_hits);
            SetCollection<edm4eic::ProtoCluster>(GetOutputTags()[0], std::move(clusters));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }

    private:
      CalorimeterTruthClustering m_algo;

};

} // eicrecon
