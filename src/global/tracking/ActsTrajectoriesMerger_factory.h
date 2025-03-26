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
  Output<const ActsExamples::Trajectories> m_acts_trajectories_output{this};

public:
  void Configure() {}

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    m_acts_trajectories_output().insert(m_acts_trajectories_output().end(),
                                        m_acts_trajectories1_input().begin(),
                                        m_acts_trajectories1_input().end());
    m_acts_trajectories_output().insert(m_acts_trajectories_output().end(),
                                        m_acts_trajectories2_input().begin(),
                                        m_acts_trajectories2_input().end());
  }
};

} // namespace eicrecon
