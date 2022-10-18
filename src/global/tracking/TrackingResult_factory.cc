// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackingResult_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/string/StringHelpers.h"
#include <JANA/JEvent.h>

void TrackingResult_factory::Init() {
    // This prefix will be used for parameters
    std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
    std::string param_prefix = plugin_name+ ":" + GetTag();

    // Set input data tags properly
    InitDataTags(param_prefix);

    // SpdlogMixin logger initialization, sets m_log
    InitLogger(param_prefix, "info");

    m_particle_maker_algo.init(m_log);
}

void TrackingResult_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void TrackingResult_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::string input_tag = GetInputTags()[0];

    // Collect all hits
    auto trajectories = event->Get<Jug::Trajectories>(input_tag);
    auto result = m_particle_maker_algo.execute(trajectories);
    Insert(result);
}
