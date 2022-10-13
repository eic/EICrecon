// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "ReconstructedParticleAssociations_factory.h"

#include <JANA/JEvent.h>

namespace eicrecon {
    void ReconstructedParticleAssociations_factory::Init() {
        // This prefix will be used for parameters
        std::string param_prefix = "Tracking:" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");

        // jana should not delete edm4eic::ReconstructedParticles from this factory
        // TrackerHits created by other factories, this factory only collect them together
        SetFactoryFlag(JFactory::NOT_OBJECT_OWNER);
    }

    void ReconstructedParticleAssociations_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

    }

    void ReconstructedParticleAssociations_factory::Process(const std::shared_ptr<const JEvent> &event) {
        auto prt_with_assoc = event->GetSingle<eicrecon::ParticlesWithAssociation>(GetInputTags()[0]);
        std::vector<edm4eic::ReconstructedParticle *> result;
        for(auto part: prt_with_assoc->particles()) {
            result.push_back(part);
        }

        Set(std::move(result));
    }
} // eicrecon