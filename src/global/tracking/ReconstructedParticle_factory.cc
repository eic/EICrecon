// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "ReconstructedParticle_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "ParticlesFromTrackFitResult.h"

namespace eicrecon {
    void ReconstructedParticle_factory::Init() {
        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string param_prefix = "GlbReco:" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");
    }

    void ReconstructedParticle_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void ReconstructedParticle_factory::Process(const std::shared_ptr<const JEvent> &event) {
        auto tracking_data = event->GetSingle<ParticlesFromTrackFitResult>("CentralTrackingParticles");
        std::vector<edm4eic::ReconstructedParticle*> result;
        for(size_t i=0; i < tracking_data->particles()->size(); i++) {
            auto particle = (*tracking_data->particles())[i];
            result.push_back(new edm4eic::ReconstructedParticle(particle));
        }
        Set(result);
    }
} // eicrecon