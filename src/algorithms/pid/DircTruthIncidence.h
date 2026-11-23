// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 hpDIRC GR effort
//
// Truth-level bar incidence for the hpDIRC geometric reconstruction (DircGR).
//
// Input: DIRC quartz-bar hits (DIRCHits with surface=1, i.e. DIRCQuartzHits).
// For every charged MCParticle that is not an optical photon, not flagged
// producedBySecondary and fast enough to radiate, take its EARLIEST quartz
// hit (= first bar entered) and correct the DD4hep midpoint quantities back to
// the bar entry face:
//   x_in = x_mid - u * L/2          (u = hit momentum direction)
//   t_in = t_mid - (L/4) / (beta c) (DD4hep TrackerWeighted time convention)
// Validated against ddDIRCactionStep in Phase 1 (phase1/p1ana.C, "NEW").
//
// Outputs (index-aligned):
//   - TrackSegment with one TrackPoint at the entry. TrackPoint::surface holds
//     the quartz-hit cellID (module/section/bar decodable with the DIRCHits
//     readout), TrackSegment::length the path length L in the bar.
//   - subset collection of the chosen quartz hits (truth link to MCParticle).

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/pid/DircTruthIncidenceConfig.h"

namespace eicrecon {

using DircTruthIncidenceAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<edm4eic::TrackSegmentCollection,
                                             edm4hep::SimTrackerHitCollection>>;

class DircTruthIncidence : public DircTruthIncidenceAlgorithm,
                           public WithPodConfig<DircTruthIncidenceConfig> {

public:
  DircTruthIncidence(std::string_view name)
      : DircTruthIncidenceAlgorithm{name,
                                    {"inputQuartzHits"},
                                    {"outputTrackSegments", "outputIncidenceHits"},
                                    "Truth bar-entry track segments for the hpDIRC GR."} {}

  void init() final;
  void process(const Input&, const Output&) const final;
};

} // namespace eicrecon
