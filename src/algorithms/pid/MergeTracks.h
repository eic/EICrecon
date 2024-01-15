// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// merge together TrackSegments, sorting their TrackPoints by time
/* FIXME: only VectorMember `points` is combined, which is all that is needed
 * for the RICH detectors. If using this algorithm for any other purpose, you
 * may want to combine the other members and relations in `TrackSegment`.
 */

#pragma once

// data model
#include <algorithms/algorithm.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <vector>

namespace eicrecon {

  using MergeTracksAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      std::vector<const edm4eic::TrackSegmentCollection>
    >,
    algorithms::Output<
      edm4eic::TrackSegmentCollection
    >
  >;

  class MergeTracks
  : public MergeTracksAlgorithm {

  public:
    MergeTracks(std::string_view name)
      : MergeTracksAlgorithm{name,
                            {"inputTrackSegments"},
                            {"outputTrackSegments"},
                            "Effecitvely 'zip' the input track segments."} {}

    void init(std::shared_ptr<spdlog::logger>& logger);
    void process(const Input&, const Output&) const final;

    private:
      std::shared_ptr<spdlog::logger> m_log;

  };
}
