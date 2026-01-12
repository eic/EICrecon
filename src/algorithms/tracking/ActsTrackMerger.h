// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <algorithms/algorithm.h>
#include <string>
#include <string_view>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using ActsTrackMergerAlgorithm =
    algorithms::Algorithm<algorithms::Input<std::vector<const Acts::ConstVectorMultiTrajectory*>,
                                            std::vector<const Acts::ConstVectorTrackContainer*>,
                                            std::vector<const Acts::ConstVectorMultiTrajectory*>,
                                            std::vector<const Acts::ConstVectorTrackContainer*>>,
                          algorithms::Output<std::vector<Acts::ConstVectorMultiTrajectory*>,
                                             std::vector<Acts::ConstVectorTrackContainer*>>>;

class ActsTrackMerger : public ActsTrackMergerAlgorithm, public WithPodConfig<NoConfig> {
public:
  ActsTrackMerger(std::string_view name)
      : ActsTrackMergerAlgorithm{name,
                                 {
                                     "inputActsTrackStates1",
                                     "inputActsTracks1",
                                     "inputActsTrackStates2",
                                     "inputActsTracks2",
                                 },
                                 {
                                     "outputActsTrackStates",
                                     "outputActsTracks",
                                 },
                                 "Merges two Acts track container inputs into one"} {};

  void init() final {};
  void process(const Input&, const Output&) const final;

  // Helper method that returns the merged result
  static std::tuple<std::vector<Acts::ConstVectorMultiTrajectory*>,
             std::vector<Acts::ConstVectorTrackContainer*>>
  merge(const std::vector<const Acts::ConstVectorMultiTrajectory*>& input_track_states1,
        const std::vector<const Acts::ConstVectorTrackContainer*>& input_tracks1,
        const std::vector<const Acts::ConstVectorMultiTrajectory*>& input_track_states2,
        const std::vector<const Acts::ConstVectorTrackContainer*>& input_tracks2) ;
};

} // namespace eicrecon
