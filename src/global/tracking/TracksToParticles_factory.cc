// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TracksToParticles_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/string/StringHelpers.h"
#include <JANA/JEvent.h>

void TracksToParticles_factory::Init() {

    // SpdlogMixin logger initialization, sets m_log
    InitLogger(GetPrefix(), "info");

    m_particle_maker_algo.init(m_log);
}

void TracksToParticles_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {

}

void TracksToParticles_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::string input_tag = GetInputTags()[0];

    try {
        // Collect all hits
        auto tracks = event->Get<edm4eic::Track>(input_tag);
        auto result = m_particle_maker_algo.execute(tracks);
        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::move(result));
        
    }
    catch(std::exception &e) {
        throw JException(e.what());
    }
}
