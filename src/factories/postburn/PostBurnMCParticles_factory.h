// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Alex Jentsch, Jihee Kim, Brian Page
//

#include "algorithms/postburn/PostBurn.h"
#include "algorithms/postburn/PostBurnConfig.h"

// Event Model related classes
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>

#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class PostBurnMCParticles_factory :
    public JOmniFactory<PostBurnMCParticles_factory, PostBurnConfig> {

public:
    using AlgoT = eicrecon::PostBurn;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4hep::MCParticle> m_mcparts_input {this};
    PodioOutput<edm4hep::MCParticle> m_postburn_output {this};

    ParameterRef<bool>      pidAssumePionMass{this, "pidAssumePionMass", config().pidAssumePionMass};
    ParameterRef<double>    crossingAngle{this, "crossingAngle", config().crossingAngle};
    ParameterRef<double>    pidPurity{this, "pidPurity", config().pidPurity};
    ParameterRef<bool>      correctBeamFX       {this, "correctBeamFX", config().correctBeamFX};
    ParameterRef<bool>      pidUseMCTruth         {this, "pidUseMCTruth", config().pidUseMCTruth};


public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->applyConfig(config());
		m_algo->init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_algo->process({m_mcparts_input()}, {m_postburn_output().get()});
    }

};

} // eicrecon
