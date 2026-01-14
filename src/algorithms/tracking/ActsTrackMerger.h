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
    algorithms::Algorithm<algorithms::Input<std::vector<const ActsExamples::ConstTrackContainer*>,
                                            std::vector<const ActsExamples::ConstTrackContainer*>>,
                          algorithms::Output<std::vector<ActsExamples::ConstTrackContainer*>>>;

class ActsTrackMerger : public ActsTrackMergerAlgorithm, public WithPodConfig<NoConfig> {
public:
  ActsTrackMerger(std::string_view name)
      : ActsTrackMergerAlgorithm{name,
                                 {
                                     "inputActsTracks1",
                                     "inputActsTracks2",
                                 },
                                 {
                                     "outputActsTracks",
                                 },
                                 "Merges two ActsExamples::ConstTrackContainer inputs into one"} {};

  void init() final {};
  void process(const Input&, const Output&) const final;

  // Helper method that returns the merged result
  std::vector<ActsExamples::ConstTrackContainer*>
  merge(const std::vector<const ActsExamples::ConstTrackContainer*>& input1,
        const std::vector<const ActsExamples::ConstTrackContainer*>& input2) const;
};

} // namespace eicrecon
