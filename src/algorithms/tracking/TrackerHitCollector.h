// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Whitney Armstrong, Sylvester Joosten, Chao Peng, Wouter Deconinck

#pragma once

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/TrackerHitCollection.h>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  /** Collect the tracking hits into a single collection.
   *
   * \ingroup reco
   */
  class TrackerHitCollector : public WithPodConfig<NoConfig> {

  protected:

    std::shared_ptr<spdlog::logger> m_log;

  public:

    void init(std::shared_ptr<spdlog::logger>& logger) {
        m_log = logger;
    }

    std::unique_ptr<edm4eic::TrackerHitCollection> process(
        const std::vector<const edm4eic::TrackerHitCollection*>& hit_collections
    ) {

      auto hits = std::make_unique<edm4eic::TrackerHitCollection>();
      hits->setSubsetCollection();

      for (const auto* hit_collection : hit_collections) {
        for (const auto& hit : *hit_collection) {
          hits->push_back(hit);
        }
      }

      return std::move(hits);
    }

  };

} // namespace eicrecon
