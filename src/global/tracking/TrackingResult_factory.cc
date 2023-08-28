// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackingResult_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <JANA/JEvent.h>

void TrackingResult_factory::Init() {

    // SpdlogMixin logger initialization, sets m_log
    InitLogger(GetApplication(), GetPrefix(), "info");

    m_particle_maker_algo.init(m_log);
}

void TrackingResult_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {

}

void TrackingResult_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::string input_tag = GetInputTags()[0];

    try {
        // Collect all hits
        auto trajectories = event->Get<ActsExamples::Trajectories>(input_tag);
        auto track_params = m_particle_maker_algo.execute(trajectories);
        SetCollection<edm4eic::TrackParameters>(GetOutputTags()[0], std::move(track_params));
    }
    catch(std::exception &e) {
        throw JException(e.what());
    }
}
