// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include <string>

#include "algorithms/fardetectors/FarForwardHitExtractor.h"
#include "algorithms/fardetectors/FarForwardHitExtractorConfig.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class FarForwardHitExtractor_factory :
    public JOmniFactory<FarForwardHitExtractor_factory, FarForwardHitExtractorConfig> {

public:
    using AlgoT = eicrecon::FarForwardHitExtractor;
private:
    std::unique_ptr<AlgoT> m_algo;
    PodioInput<edm4hep::MCParticle> m_in_mc_particles {this};
    PodioInput<edm4eic::TrackerHit> m_in_reco_particles {this};

    PodioOutput<edm4eic::ReconstructedParticle> m_out_reco_particles {this};


    Service<DD4hep_service> m_geoSvc {this};

    ParameterRef<double> plane1_min_z{this, "plane1_min_z", config().plane1_min_z};
    ParameterRef<double> plane1_max_z{this, "plane1_max_z", config().plane1_max_z};
    ParameterRef<double> plane2_min_z{this, "plane2_min_z", config().plane2_min_z};
    ParameterRef<double> plane2_max_z{this, "plane2_max_z", config().plane2_max_z};


public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>();
        m_algo->init(logger());
        m_algo->applyConfig(config());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        auto output = m_algo->execute(
          m_in_mc_particles(),
          m_in_reco_particles()
        );
        m_out_reco_particles() = std::move(output);
    }

};

} // eicrecon
