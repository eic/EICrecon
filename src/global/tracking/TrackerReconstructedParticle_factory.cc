// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "TrackerReconstructedParticle_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include "extensions/string/StringHelpers.h"

namespace eicrecon {
    void TrackerReconstructedParticle_factory::Init() {
        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
        std::string param_prefix = plugin_name+ ":" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");
        auto level = m_log->level();
        m_log->info("Log level {} set to {}", param_prefix, level);
    }

    void TrackerReconstructedParticle_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void TrackerReconstructedParticle_factory::Process(const std::shared_ptr<const JEvent> &event) {
        auto tracking_data = event->GetSingle<ParticlesFromTrackFitResult>("CentralTrackingParticles");
        std::vector<edm4eic::ReconstructedParticle*> result;
        for(size_t i=0; i < tracking_data->particles()->size(); i++) {
            auto particle = (*tracking_data->particles())[i];
            result.push_back(new edm4eic::ReconstructedParticle(particle));
        }
        Set(result);
    }
} // eicrecon