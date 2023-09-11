// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>
#include <algorithms/fardetectors/FarDetectorLinearProjection.h>

#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

    class FarDetectorLinearProjection_factoryT :
    public JChainMultifactoryT<FarDetectorLinearProjectionConfig>,
    public SpdlogMixin {

    public:

      explicit FarDetectorLinearProjection_factoryT(
          std::string tag,
          const std::vector<std::string>& input_tags,
          const std::vector<std::string>& output_tags,
          FarDetectorLinearProjectionConfig cfg):
          JChainMultifactoryT(std::move(tag), input_tags, output_tags, cfg) {

          DeclarePodioOutput<edm4eic::TrackParameters>(GetOutputTags()[0]);
      }

        /** One time initialization **/
        void Init() override {
          auto app = GetApplication();

          // SpdlogMixin logger initialization, sets m_log
          InitLogger(app, GetPrefix(), "info");

          auto cfg = GetDefaultConfig();

          m_reco_algo.applyConfig(cfg);
          m_reco_algo.init(logger());

        }

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override {

          auto inputhits = static_cast<const edm4eic::TrackSegmentCollection*>(event->GetCollectionBase(GetInputTags()[0]));

          try {
            auto outputTracks = m_reco_algo.produce(*inputhits);
            SetCollection<edm4eic::TrackParameters>(GetOutputTags()[0],std::move(outputTracks));
          }
          catch(std::exception &e) {
            throw JException(e.what());
          }

        }

    private:

        eicrecon::FarDetectorLinearProjection m_reco_algo;        // Actual digitisation algorithm

    };

} // eicrecon
