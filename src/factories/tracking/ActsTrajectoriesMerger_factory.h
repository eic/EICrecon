// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <ActsExamples/EventData/Trajectories.hpp>
#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class ActsTrajectoriesMerger_factory : public JOmniFactory<ActsTrajectoriesMerger_factory> {
private:
  Input<ActsExamples::Trajectories> m_acts_trajectories1_input{this};
  Input<ActsExamples::Trajectories> m_acts_trajectories2_input{this};
  Output<ActsExamples::Trajectories> m_acts_trajectories_output{this};

public:
  void Configure() {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {

    // FIXME clear output since it may not have been initialized or reset
    m_acts_trajectories_output().clear();

    for (const auto& traj : m_acts_trajectories1_input()) {
      ActsExamples::Trajectories::IndexedParameters trackParameters;
      for (auto tip : traj->tips()) {
        trackParameters.insert({tip, traj->trackParameters(tip)});
      }
      m_acts_trajectories_output().push_back(
          new ActsExamples::Trajectories(traj->multiTrajectory(), traj->tips(), trackParameters));
    }
    for (const auto& traj : m_acts_trajectories2_input()) {
      ActsExamples::Trajectories::IndexedParameters trackParameters;
      for (auto tip : traj->tips()) {
        trackParameters.insert({tip, traj->trackParameters(tip)});
      }
      m_acts_trajectories_output().push_back(
          new ActsExamples::Trajectories(traj->multiTrajectory(), traj->tips(), trackParameters));
    }
  }
};

} // namespace eicrecon
