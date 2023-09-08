// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once
#include "services/geometry/dd4hep/JDD4hep_service.h"

// Event Model related classes
#include <edm4hep/TrackerHitCollection.h>
#include <edm4eic/RawTrackerHit.h>

#include "algorithms/fardetectors/FarDetectorTrackerCluster.h"

#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

  class FarDetectorTrackerCluster_factoryT :
  public JChainMultifactoryT<FarDetectorTrackerClusterConfig>,
  public SpdlogMixin {

  public:

    explicit FarDetectorTrackerCluster_factoryT(
          std::string tag,
          const std::vector<std::string>& input_tags,
          const std::vector<std::string>& output_tags,
          FarDetectorTrackerClusterConfig cfg ) :
          JChainMultifactoryT<FarDetectorTrackerClusterConfig>(std::move(tag),input_tags,output_tags,cfg ) {

      DeclarePodioOutput<edm4hep::TrackerHit>(GetOutputTags()[0]);

    }


      /** One time initialization **/
      void Init() override {

        auto app = GetApplication();

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(app, GetPrefix(), "info");

        auto cfg = GetDefaultConfig();

        auto geoSvc = app->GetService<JDD4hep_service>();

        // This prefix will be used for parameters
        std::string param_prefix = GetPluginName() + ":" + GetTag();

        japp->SetDefaultParameter(param_prefix+":hit_time_limit", cfg.time_limit,"Time limit for adding a hit to a cluster [ns]");

        // Setup algorithm
        m_reco_algo.applyConfig(cfg);
        m_reco_algo.init(geoSvc->detector(),geoSvc->cellIDPositionConverter(),logger());

      }

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override {
        auto inputhits  = static_cast<const edm4eic::RawTrackerHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        try {
          auto outputclusters = m_reco_algo.produce(*inputhits);
          SetCollection<edm4hep::TrackerHit>(GetOutputTags()[0],std::move(outputclusters));
        }
        catch(std::exception &e) {
          throw JException(e.what());
        }
      }


  private:
      eicrecon::FarDetectorTrackerCluster m_reco_algo;        // Actual digitisation algorithm

  };

} // eicrecon
