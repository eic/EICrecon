// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <ActsExamples/EventData/Trajectories.hpp>
#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/ActsExamplesEdm.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <typename edm_t = eicrecon::ActsExamplesEdm>
class ActsTrajectoriesMerger_factory : public JOmniFactory<ActsTrajectoriesMerger_factory<edm_t>> {
private:
  using FactoryT = JOmniFactory<ActsTrajectoriesMerger_factory<edm_t>>;

  template <typename T> using PodioInput   = typename FactoryT::template PodioInput<T>;
  template <typename T> using PodioOutput  = typename FactoryT::template PodioOutput<T>;
  template <typename T> using Input        = typename FactoryT::template Input<T>;
  template <typename T> using Output       = typename FactoryT::template Output<T>;
  template <typename T> using ParameterRef = typename FactoryT::template ParameterRef<T>;
  template <typename T> using Service      = typename FactoryT::template Service<T>;

  Input<typename edm_t::Trajectories> m_acts_trajectories1_input{this};
  Input<typename edm_t::Trajectories> m_acts_trajectories2_input{this};
  Output<typename edm_t::Trajectories> m_acts_trajectories_output{this};

public:
  void Configure() {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    for (const auto& traj : m_acts_trajectories1_input()) {
      typename edm_t::Trajectories::IndexedParameters trackParameters;
      for (auto tip : traj->tips()) {
        trackParameters.insert({tip, traj->trackParameters(tip)});
      }
      m_acts_trajectories_output().push_back(
          new edm_t::Trajectories(traj->multiTrajectory(), traj->tips(), trackParameters));
    }
    for (const auto& traj : m_acts_trajectories2_input()) {
      typename edm_t::Trajectories::IndexedParameters trackParameters;
      for (auto tip : traj->tips()) {
        trackParameters.insert({tip, traj->trackParameters(tip)});
      }
      m_acts_trajectories_output().push_back(
          new edm_t::Trajectories(traj->multiTrajectory(), traj->tips(), trackParameters));
    }
  }
};

} // namespace eicrecon
