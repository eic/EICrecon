// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "MergeTracks.h"

#include <edm4eic/MutableTrackSegment.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegment.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <spdlog/logger.h>
#include <algorithm>
#include <cstddef>
#include <exception>

// AlgorithmInit
//---------------------------------------------------------------------------
void eicrecon::MergeTracks::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger)
{
  m_log = logger;
}


// AlgorithmChangeRun
//---------------------------------------------------------------------------
void eicrecon::MergeTracks::AlgorithmChangeRun() {
}


// AlgorithmProcess
//---------------------------------------------------------------------------
std::unique_ptr<edm4eic::TrackSegmentCollection> eicrecon::MergeTracks::AlgorithmProcess(
    std::vector<const edm4eic::TrackSegmentCollection*> in_track_collections
    )
{
  // logging
  m_log->trace("{:=^70}"," call MergeTracks::AlgorithmProcess ");

  // start output collection
  auto out_tracks = std::make_unique<edm4eic::TrackSegmentCollection>();

  // check that all input collections have the same size
  std::size_t n_tracks = -1;
  for(const auto& in_track_collection : in_track_collections) {
    if(n_tracks == -1)
      n_tracks = in_track_collection->size();
    else if(n_tracks != in_track_collection->size()) {
      m_log->error("input track collections do not have the same size; cannot merge");
      return out_tracks;
    }
  }

  // loop over track collection elements
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

  return out_tracks;
}
