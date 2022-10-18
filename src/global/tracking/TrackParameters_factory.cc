// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include "TrackParameters_factory.h"
#include "extensions/string/StringHelpers.h"

#include <JANA/JEvent.h>

#include <edm4eic/TrackParameters.h>

#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

namespace eicrecon {
    void TrackParameters_factory::Init() {
        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
        std::string param_prefix = plugin_name+ ":" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");
    }

    void TrackParameters_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void TrackParameters_factory::Process(const std::shared_ptr<const JEvent> &event) {
        auto tracking_data = event->GetSingle<ParticlesFromTrackFitResult>("CentralTrackingParticles");
        std::vector<edm4eic::TrackParameters*> result;
        for(size_t i=0; i < tracking_data->trackParameters()->size(); i++) {
            auto track_params = (*tracking_data->trackParameters())[i];
            result.push_back(new edm4eic::TrackParameters(track_params));
        }
        Set(result);
    }
} // eicrecon