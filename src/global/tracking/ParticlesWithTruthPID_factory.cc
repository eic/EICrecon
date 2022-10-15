// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>

#include <edm4hep/MCParticle.h>
#include <edm4hep/MCParticleObj.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/TrackParametersObj.h>

#include "ParticlesWithTruthPID_factory.h"
#include <algorithms/reco/ParticlesWithAssociation.h>

#include <extensions/podio_access/accessor.h>

using MCParticleObjPtr = edm4hep::MCParticleObj*;
using TrackParametersObjPtr = edm4eic::TrackParametersObj*;
ALLOW_ACCESS(edm4hep::MCParticle, m_obj, MCParticleObjPtr);
ALLOW_ACCESS(edm4eic::TrackParameters, m_obj, TrackParametersObjPtr);

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

        // Set indexes for MCParticles. Evil, Evil, Evil is here. TODO remove ASAP
        for(size_t i=0; i< mc_particles.size(); i++) {
            // Dirty hacks begun! Mutate army of mutant particles because of PODIO mudagen
            auto * mc_particle = const_cast<edm4hep::MCParticle*>(mc_particles[i]);

            // Nothing is private in this world anymore!
            auto obj = ACCESS(*mc_particle, m_obj);
            obj->id.index = (int)i;     // Change!
        }

        // Set indexes for TrackParameters. Evil, Evil, Evil is here. TODO remove ASAP
        for(size_t i=0; i< track_params.size(); i++) {
            // Dirty hacks begun! Mutate army of mutant tracks because of PODIO mudagen
            auto * track_param = const_cast<edm4eic::TrackParameters*>(track_params[i]);

            // Nothing is private in this world anymore!
            auto obj = ACCESS(*track_param, m_obj);
            obj->id.index = (int)i;     // Change!
        }

        auto result = m_matching_algo.process(mc_particles, track_params);
        Insert(result);
    }
} // eicrecon