// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/TrackParametersCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/TrackParamTruthInit.h"
#include "algorithms/tracking/TrackParamTruthInitConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

class TrackParamTruthInit_factory :
        public JOmniFactory<TrackParamTruthInit_factory, TrackParamTruthInitConfig> {

private:
    using AlgoT = eicrecon::TrackParamTruthInit;
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4hep::MCParticle> m_particles_input {this};
    PodioOutput<edm4eic::TrackParameters> m_parameters_output {this};

    ParameterRef<double> m_maxVertexX {this, "MaxVertexX", config().m_maxVertexX , "Maximum abs(vertex x) for truth tracks turned into seed"};
    ParameterRef<double> m_maxVertexY {this, "MaxVertexY", config().m_maxVertexY , "Maximum abs(vertex y) for truth tracks turned into seed"};
    ParameterRef<double> m_maxVertexZ {this, "MaxVertexZ", config().m_maxVertexZ , "Maximum abs(vertex z) for truth tracks turned into seed"};
    ParameterRef<double> m_minMomentum {this, "MinMomentum", config().m_minMomentum , "Minimum momentum for truth tracks turned into seed"};
    ParameterRef<double> m_maxEtaForward {this, "MaxEtaForward", config().m_maxEtaForward , "Maximum forward abs(eta) for truth tracks turned into seed"};
    ParameterRef<double> m_maxEtaBackward {this, "MaxEtaBackward", config().m_maxEtaBackward , "Maximum backward abs(eta) for truth tracks turned into seed"};
    ParameterRef<double> m_momentumSmear {this, "MomentumSmear", config().m_momentumSmear, "Momentum magnitude fraction to use as width of gaussian smearing"};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>();
        m_algo->applyConfig(config());
        m_algo->init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_parameters_output() = m_algo->produce(m_particles_input());
    }
};

} // eicrecon
