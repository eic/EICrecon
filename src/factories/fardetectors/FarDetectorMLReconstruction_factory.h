// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/TrajectoryCollection.h>
#include <algorithms/fardetectors/FarDetectorMLReconstruction.h>
#include <algorithms/fardetectors/FarDetectorMLReconstructionConfig.h>

#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>
#include <Evaluator/DD4hepUnits.h>

namespace eicrecon {

class FarDetectorMLReconstruction_factory :
public JOmniFactory<FarDetectorMLReconstruction_factory,FarDetectorMLReconstructionConfig> {

private:
    eicrecon::FarDetectorMLReconstruction m_algo;

    PodioInput<edm4eic::TrackParameters>  m_trackparam_input  {this};
    PodioOutput<edm4eic::Trajectory>      m_trajectory_output {this};
    PodioOutput<edm4eic::TrackParameters> m_trackparam_output {this};

    
    ParameterRef<std::string> modelPath       {this, "modelPath",       config().modelPath       };
    ParameterRef<std::string> methodName      {this, "methodName",      config().methodName      };
    ParameterRef<std::string> fileName        {this, "fileName",        config().fileName        };
    ParameterRef<std::string> environmentPath {this, "environmentPath", config().environmentPath };

    ParameterRef<float> electron_beamE  {this, "electron_beamE",  config().electron_beamE };
    

public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        std::tie(m_trajectory_output(), m_trackparam_output()) = m_algo.process(*m_trackparam_input());
    }
  };

} // eicrecon
