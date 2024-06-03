// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Simon Gardner

#pragma once

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/TrajectoryCollection.h>
#include <algorithms/fardetectors/FarDetectorMLReconstruction.h>
#include <algorithms/fardetectors/FarDetectorMLReconstructionConfig.h>

#include "extensions/jana/JOmniFactory.h"
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>
#include <Evaluator/DD4hepUnits.h>

namespace eicrecon {

class FarDetectorMLReconstruction_factory :
        public JOmniFactory<FarDetectorMLReconstruction_factory,FarDetectorMLReconstructionConfig> {

public:
    using AlgoT = eicrecon::FarDetectorMLReconstruction;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::TrackParameters>  m_trackparam_input     {this};
    PodioInput<edm4hep::MCParticle>       m_beamelectrons_input {this};
    PodioOutput<edm4eic::Trajectory>      m_trajectory_output    {this};
    PodioOutput<edm4eic::TrackParameters> m_trackparam_output    {this};
    PodioOutput<edm4eic::Track>           m_track_output         {this};


    ParameterRef<std::string> m_modelPath       {this, "modelPath",       config().modelPath       };
    ParameterRef<std::string> m_methodName      {this, "methodName",      config().methodName      };


public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
        m_algo->applyConfig(config());
        m_algo->init();
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_algo->process({m_trackparam_input(),m_beamelectrons_input()}, {m_trajectory_output().get(), m_trackparam_output().get(), m_track_output().get()});
    }
  };

} // eicrecon
