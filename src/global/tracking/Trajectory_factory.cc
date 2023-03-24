// Created by Shyam Kumar; INFN Bari, shyam.kumar@ba.infn.it; shyam055119@gmail.com
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "Trajectory_factory.h"
#include <edm4eic/Trajectory.h>
#include "extensions/string/StringHelpers.h"
#include <JANA/JEvent.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

namespace eicrecon {
    void Trajectory_factory::Init() {
        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
        std::string param_prefix = plugin_name+ ":" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");
    }

    void Trajectory_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void Trajectory_factory::Process(const std::shared_ptr<const JEvent> &event) {

        auto trajectories = event->Get<edm4eic::Trajectory>("outputTrajectories");

        try {
            std::vector<edm4eic::Trajectory *> result;
            for (const auto *trajectory: trajectories) {
                auto trajectory_params = (*trajectory_info->Trajectory())[i];
                result.push_back(new edm4eic::Trajectory(trajectory_params));
            }
            Set(result);
        }
        catch(std::exception &e) {
            m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
        }
    }
} // eicrecon
