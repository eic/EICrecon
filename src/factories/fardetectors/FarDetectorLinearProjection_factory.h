// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>
#include <algorithms/fardetectors/FarDetectorLinearProjection.h>

#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

class FarDetectorLinearProjection_factory :
public JOmniFactory<FarDetectorLinearProjection_factory,FarDetectorLinearProjectionConfig> {

    FarDetectorLinearProjection m_algo;        // Actual digitisation algorithm

    PodioInput<edm4eic::TrackSegment>     m_hits_input    {this};
    PodioOutput<edm4eic::TrackParameters> m_tracks_output {this};

    ParameterRef<std::vector<float>>     plane_position  {this, "plane_position", config().plane_position };
    ParameterRef<std::vector<float>>     plane_a         {this, "plane_a",        config().plane_a        };
    ParameterRef<std::vector<float>>     plane_b         {this, "plane_b",        config().plane_b        };

  public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_tracks_output() = m_algo.process(*m_hits_input());
    }
  };

} // eicrecon
