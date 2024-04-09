// Created by Minjung Kim (minjung.kim@lbl.gov)
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <edm4eic/TrackParametersCollection.h>
#include "ActsExamples/EventData/Trajectories.hpp"
#include "algorithms/tracking/AmbiguitySolver.h"
#include "algorithms/tracking/AmbiguitySolverConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

class AmbiguitySolver_factory
    : public JOmniFactory<AmbiguitySolver_factory, AmbiguitySolverConfig> {

private:
  using AlgoT = eicrecon::AmbiguitySolver;
  std::unique_ptr<AlgoT> m_algo;

  Input<ActsExamples::ConstTrackContainer> m_abmiguitiy_solver_input{this};
  PodioOutput<edm4eic::TrackParameters> m_abmiguitiy_solver_trkparam_output{this};
  Output<ActsExamples::ConstTrackContainer> m_abmiguitiy_solver_output{this};
  
  ParameterRef<std::uint32_t> maximumSharedHits{this, "maxsharedcut", config().maximumSharedHits,
                                              "Maximum number of shared hits allowed"};
  ParameterRef<std::uint32_t> maximumIterations{this, "maxiterf", config().maximumIterations,
                                                 "Maximum number of iterations"};
  ParameterRef<std::size_t> nMeasurementsMin{this, "minNumMeasurements", config().nMeasurementsMin,
      "Number of measurements required for further reconstruction"};


public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>();
    m_algo->applyConfig(config());
    m_algo->init(logger());
    std::cout<<"MJ DEBUG configure started"<<std::endl;
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    std::tie(m_abmiguitiy_solver_trkparam_output(),m_abmiguitiy_solver_output()) = m_algo->process(m_abmiguitiy_solver_input());
  }
};

} // namespace eicrecon
