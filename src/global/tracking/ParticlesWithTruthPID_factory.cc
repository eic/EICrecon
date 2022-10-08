// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>

#include <edm4hep/MCParticle.h>
#include <edm4eic/TrackParameters.h>

#include "ParticlesWithTruthPID_factory.h"
#include <algorithms/reco/ParticlesWithAssociation.h>

namespace eicrecon {
    void ParticlesWithTruthPID_factory::Init() {
        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string param_prefix = "Tracking:" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");

        m_matching_algo.init(logger());
    }

    void ParticlesWithTruthPID_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void ParticlesWithTruthPID_factory::Process(const std::shared_ptr<const JEvent> &event) {
        auto mc_particles = event->Get<edm4hep::MCParticle>(GetInputTags()[0]);
        auto track_params = event->Get<edm4eic::TrackParameters>(GetInputTags()[1]);

        auto result = m_matching_algo.execute(mc_particles, track_params);
        Insert(result);
    }
} // eicrecon