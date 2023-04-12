// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackingResult_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/string/StringHelpers.h"
#include <JANA/JEvent.h>

void TrackingResult_factory::Init() {

    // SpdlogMixin logger initialization, sets m_log
    InitLogger(GetPrefix(), "info");

    m_particle_maker_algo.init(m_log);
}

void TrackingResult_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {

}

void TrackingResult_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::string input_tag = GetInputTags()[0];

    try {
        // Collect all hits
        auto trajectories = event->Get<eicrecon::TrackingResultTrajectory>(input_tag);
        auto result = m_particle_maker_algo.execute(trajectories);
        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::move(result.first));
        SetCollection<edm4eic::TrackParameters>(GetOutputTags()[1], std::move(result.second));
    }
    catch(std::exception &e) {
        m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
    }
}
