// SPDX-License-Identifier: JSA
// Copyright (C) 2023, Christopher Dilks

#include "MergeTracks.h"

#include <edm4eic/TrackPoint.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <unordered_map>
#include <utility>

namespace eicrecon {

void MergeTracks::init(std::shared_ptr<spdlog::logger>& logger)
{
  m_log = logger;
}

void MergeTracks::process(
    const MergeTracks::Input& input,
    const MergeTracks::Output& output) const {

  const auto [in_track_collections] = input;
  auto [out_tracks] = output;

  // logging
  m_log->trace("{:=^70}"," call MergeTracks::AlgorithmProcess ");

  // check that all input collections have the same size
  std::unordered_map<std::size_t, std::size_t> in_track_collection_size_distribution;
  for(const auto& in_track_collection : in_track_collections) {
    ++in_track_collection_size_distribution[in_track_collection->size()];
  }
  if (in_track_collection_size_distribution.size() != 1) {
    std::vector<size_t> in_track_collection_sizes;
    std::transform(in_track_collections.begin(), in_track_collections.end(),
      std::back_inserter(in_track_collection_sizes),
      [](const auto& in_track_collection) { return in_track_collection->size(); });
    m_log->error("cannot merge input track collections with different sizes {}", fmt::join(in_track_collection_sizes, ", "));
    return;
  }

  // loop over track collection elements
  std::size_t n_tracks = in_track_collection_size_distribution.begin()->first;
  for(std::size_t i_track=0; i_track<n_tracks; i_track++) {

    // create a new output track, and a local container to hold its track points
    auto out_track = out_tracks->create();
    std::vector<edm4eic::TrackPoint> out_track_points;

    // loop over collections for this track, and add each track's points to `out_track_points`
    for(const auto& in_track_collection : in_track_collections) {
      const auto& in_track = in_track_collection->at(i_track);
      for(const auto& point : in_track.getPoints())
        out_track_points.push_back(point);
    }

    // sort all `out_track_points` by time
    std::sort(
        out_track_points.begin(),
        out_track_points.end(),
        [] (edm4eic::TrackPoint& a, edm4eic::TrackPoint& b) { return a.time < b.time; }
        );

    // add these sorted points to `out_track`
    for(const auto& point : out_track_points)
      out_track.addToPoints(point);

    /* FIXME: merge other members, such as `length` and `lengthError`;
     * currently not needed for RICH tracks, so such members are left as default
     */

  } // end loop over tracks
}

} // namespace eicrecon
