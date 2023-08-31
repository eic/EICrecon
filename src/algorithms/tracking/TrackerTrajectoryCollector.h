// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Whitney Armstrong, Sylvester Joosten, Chao Peng, Wouter Deconinck

#pragma once

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/TrajectoryCollection.h>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  /** Collect the tracking hits into a single collection.
   *
   * \ingroup reco
   */
  class TrackerTrajectoryCollector : public WithPodConfig<NoConfig> {

  protected:

    std::shared_ptr<spdlog::logger> m_log;

  public:

    void init(std::shared_ptr<spdlog::logger>& logger) {
        m_log = logger;
    }

    std::unique_ptr<edm4eic::TrajectoryCollection> process(
        const std::vector<const edm4eic::TrajectoryCollection*>& trajectory_collections
    ) {

      auto trajectories = std::make_unique<edm4eic::TrajectoryCollection>();
      trajectories->setSubsetCollection();

      for (const auto* trajectory_collection : trajectory_collections) {
        for (const auto& trajectory : *trajectory_collection) {
          trajectories->push_back(trajectory);
        }
      }

      return std::move(trajectories);
    }

  };

} // namespace eicrecon
