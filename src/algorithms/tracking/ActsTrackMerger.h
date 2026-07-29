// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using ActsTrackMergerAlgorithm = algorithms::Algorithm<
    algorithms::Input<Acts::ConstVectorMultiTrajectory, Acts::ConstVectorTrackContainer,
                      Acts::ConstVectorMultiTrajectory, Acts::ConstVectorTrackContainer>,
    algorithms::Output<Acts::ConstVectorMultiTrajectory*, Acts::ConstVectorTrackContainer*>>;

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
};

} // namespace eicrecon
