// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <spdlog/logger.h>
#include <edm4eic/TrackerHit.h>
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include "algorithms/tracking/TrackerHitCollector.h"

namespace eicrecon {

/// This factory just collects reconstructed hits edm4eic::TrackerHit from different sources
/// And makes a single array out of them
class TrackerHitCollector_factory :
    public JChainMultifactoryT<NoConfig>,
    public SpdlogMixin {

  public:
    explicit TrackerHitCollector_factory(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags)
    : JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {

      bool owns_data = false; // this produces a subset collection
      DeclarePodioOutput<edm4eic::TrackerHit>(GetOutputTags()[0], owns_data);

    }

    /** One time initialization **/
    void Init() override {
      auto app = GetApplication();

      // SpdlogMixin logger initialization, sets m_log
      InitLogger(app, GetPrefix(), "info");

      m_algo.init(logger());
    }

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override {

      std::vector<const edm4eic::TrackerHitCollection*> hit_collections;
      for (const auto& input_tag : GetInputTags()) {
        // TODO: we could avoid exceptions here by using JEvent::GetAllCollectionNames,
        // but until that returns a std::set and we can use c++20 contains, there is
        // not that much benefit to that approach since it requires searching through
        // a std::vector of names.
        try {
          hit_collections.emplace_back(static_cast<const edm4eic::TrackerHitCollection*>(event->GetCollectionBase(input_tag)));
        }
        catch(std::exception &e) {
          // ignore missing collections, but notify them in debug mode
          m_log->debug(e.what());
        }
      }

      try {
        auto hits = m_algo.process(hit_collections);
        SetCollection<edm4eic::TrackerHit>(GetOutputTags()[0], std::move(hits));
      }
      catch(std::exception &e) {
        throw JException(e.what());
      }
    }

  private:
    TrackerHitCollector m_algo;

  };

} // eicrecon
