// Created by Minjung Kim (minjung.kim@lbl.gov)
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp"
#include "AmbiguitySolverConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <spdlog/logger.h>

namespace eicrecon {

/** Fitting algorithm implementation .
 *
 * \ingroup tracking
 */

class AmbiguitySolver : public WithPodConfig<eicrecon::AmbiguitySolverConfig> {
public:
  AmbiguitySolver();

  void init(std::shared_ptr<spdlog::logger> log);

  // void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
  //           std::shared_ptr<spdlog::logger> log);

  /* std::tuple<std::unique_ptr<edm4eic::TrajectoryCollection>,
              std::unique_ptr<edm4eic::TrackParametersCollection>,
              std::vector<ActsExamples::Trajectories*>,
              std::vector<ActsExamples::ConstTrackContainer*>>
   */
  std::tuple<std::unique_ptr<edm4eic::TrajectoryCollection>,
             std::unique_ptr<edm4eic::TrackParametersCollection>,
             std::unique_ptr<edm4eic::TrackCollection>, std::vector<ActsExamples::Trajectories*>,
             std::vector<ActsExamples::ConstTrackContainer*>>
  process(std::vector<const ActsExamples::ConstTrackContainer*> acts_params);

private:
  std::shared_ptr<spdlog::logger> m_log;
  Acts::GreedyAmbiguityResolution::Config m_acts_cfg;
  std::unique_ptr<Acts::GreedyAmbiguityResolution> m_core;
  /// Private access to the logging instance
  std::shared_ptr<const Acts::Logger> m_acts_logger{nullptr};
  const Acts::Logger& logger() const { return *m_acts_logger; }
};

} // namespace eicrecon
