// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

/* Use MC Cherenkov photon emission points to create track points; these are called
 * pseudo-tracks, since they do not come from the track reconstruction
 */

#pragma once

#include <functional>

// data model
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>

// EICrecon
#include "PseudoTrackConfig.h"
#include <algorithms/interfaces/WithPodConfig.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

  class PseudoTrack : public WithPodConfig<PseudoTrackConfig> {

    public:
      PseudoTrack() = default;
      ~PseudoTrack() {}

      void AlgorithmInit(
          std::function<bool(double,double,double)> within_radiator,
          std::shared_ptr<spdlog::logger>& logger
          );
      void AlgorithmChangeRun();

      std::vector<edm4eic::TrackSegment*> AlgorithmProcess(
          std::vector<const edm4hep::SimTrackerHit*>& in_hits
          );

    private:
      std::shared_ptr<spdlog::logger> m_log;
      std::function<bool(double,double,double)> m_within_radiator;
  };
}
