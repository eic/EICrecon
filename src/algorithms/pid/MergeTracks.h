// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// merge together TrackSegments, sorting their TrackPoints by time
/* FIXME: only VectorMember `points` is combined, which is all that is needed
 * for the RICH detectors. If using this algorithm for any other purpose, you
 * may want to combine the other members and relations in `TrackSegment`.
 */

#pragma once

// data model
#include <edm4eic/TrackSegmentCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <vector>

namespace eicrecon {

  class MergeTracks {

    public:
      MergeTracks() = default;
      ~MergeTracks() {}

      void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
      void AlgorithmChangeRun();

      // AlgorithmProcess
      // - input: a list of TrackSegment collections
      // - output: the merged TrackSegment collections, effectively the "zip" of the input collections
      std::unique_ptr<edm4eic::TrackSegmentCollection> AlgorithmProcess(
          std::vector<const edm4eic::TrackSegmentCollection*> in_track_collections
          );

    private:
      std::shared_ptr<spdlog::logger> m_log;

  };
}
