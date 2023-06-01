// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <vector>

#include <edm4eic/TrackParametersCollection.h>
#include <JANA/JEvent.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>

#include "TruthTrackSeeding_factory.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "extensions/string/StringHelpers.h"

void eicrecon::TruthTrackSeeding_factory::Init() {
    // This prefix will be used for parameters
    std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
    std::string param_prefix = plugin_name+ ":" + GetTag();

    // Create plugin level sub-log
    m_log = spdlog::stdout_color_mt("TruthTrackSeeding_factory");

    // Ask service locator for parameter manager. We want to get this plugin parameters.
    auto pm = this->GetApplication()->GetJParameterManager();

    pm->SetDefaultParameter(param_prefix + ":verbose", m_verbose, "verbosity: 0 - none, 1 - default, 2 - debug, 3 - trace");
    pm->SetDefaultParameter(param_prefix + ":InputTags", m_input_tags, "Input data tag name");

    // This level will work for this plugin only
    switch (m_verbose) {
        case 0:
            m_log->set_level(spdlog::level::warn); break;
        case 2:
            m_log->set_level(spdlog::level::debug); break;
        case 3:
            m_log->set_level(spdlog::level::trace); break;
        default:
            m_log->set_level(spdlog::level::info); break;
    }

    m_truth_track_seeding_algo.init();
}

void eicrecon::TruthTrackSeeding_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::TruthTrackSeeding_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::vector<std::string> &input_tags = m_input_tags;
    if(input_tags.empty()) {
        input_tags = GetDefaultInputTags();
    }

    // Get MCParticles
    auto mc_particles = event->Get<edm4hep::MCParticle>(input_tags[0]);

    // Produce track parameters out of MCParticles
    std::vector<edm4eic::TrackParameters*> results;
    for(auto mc_particle: mc_particles) {

        // Only stable particles from MC
        if(mc_particle->getGeneratorStatus() != 1 ) continue;

        // Do conversion
        auto result = m_truth_track_seeding_algo.produce(mc_particle);
        results.push_back(result);

        // >oO debug output
        if(m_log->level() <= spdlog::level::debug) {
            const auto p = std::hypot(mc_particle->getMomentum().x, mc_particle->getMomentum().y, mc_particle->getMomentum().z);
            const auto charge = result->getCharge();
            m_log->debug("Invoke track finding seeded by truth particle with:");
            m_log->debug("   p =  {} GeV\"", p);
            m_log->debug("   charge = {}", charge);
            m_log->debug("   q/p =  {}", charge / p);
        }
    }

    Set(results);
}
