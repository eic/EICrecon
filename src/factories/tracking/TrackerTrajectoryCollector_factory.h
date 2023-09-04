// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/logger.h>
#include <edm4eic/TrajectoryCollection.h>
#include <extensions/jana/JChainMultifactoryT.h>
#include "algorithms/tracking/TrackerTrajectoryCollector.h"

namespace eicrecon {

    /// This factory just collects reconstructed trajectories edm4eic::Trajectory from different sources
    /// And makes a single array out of them
  class TrackerTrajectoryCollector_factory :
  public JChainMultifactoryT<NoConfig>,
    public SpdlogMixin {

    public:
      explicit TrackerTrajectoryCollector_factory(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags)
	: JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {

	bool owns_data = false; // this produces a subset collection
	DeclarePodioOutput<edm4eic::Trajectory>(GetOutputTags()[0], owns_data);

      }

    /** One time initialization **/
    void Init() override {
      auto app = GetApplication();

      // SpdlogMixin logger initialization, sets m_log
      InitLogger(app, GetPrefix(), "info");

      m_algo.init(m_log);
    }

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override {

      std::vector<const edm4eic::TrajectoryCollection*> hit_collections;
      for (const auto& input_tag : GetInputTags()) {
        // TODO: we could avoid exceptions here by using JEvent::GetAllCollectionNames,
        // but until that returns a std::set and we can use c++20 contains, there is
        // not that much benefit to that approach since it requires searching through
        // a std::vector of names.
        try {
          hit_collections.emplace_back(static_cast<const edm4eic::TrajectoryCollection*>(event->GetCollectionBase(input_tag)));
        }
        catch(std::exception &e) {
          // ignore missing collections, but notify them in debug mode
          m_log->debug(e.what());
        }
      }

      try {
        auto hits = m_algo.process(hit_collections);
        SetCollection<edm4eic::Trajectory>(GetOutputTags()[0], std::move(hits));
      }
      catch(std::exception &e) {
        throw JException(e.what());
      }
    }

    private:
	TrackerTrajectoryCollector m_algo;

    };

} // eicrecon
