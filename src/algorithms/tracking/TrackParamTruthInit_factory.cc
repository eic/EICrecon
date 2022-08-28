// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <vector>

#include "TrackParamTruthInit_factory.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>
#include <JANA/JEvent.h>

#include "TrackParamTruthInitConfig.h"

void eicrecon::TrackParamTruthInit_factory::Init() {
    // This prefix will be used for parameters
    std::string param_prefix = "TrkHitReco_" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

    // Create plugin level sub-log
    m_log = spdlog::stdout_color_mt("TrackParamTruthInit_factory");

    // Ask service locator for parameter manager. We want to get this plugin parameters.
    auto pm = this->GetApplication()->GetJParameterManager();

    pm->SetDefaultParameter(param_prefix + ":verbose", m_verbose, "verbosity: 0 - none, 1 - default, 2 - debug, 3 - trace");
    pm->SetDefaultParameter(param_prefix + ":input_tags", m_input_tags, "Input data tag name");



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

    m_truth_track_seeding_algo.init(m_log);
}

void eicrecon::TrackParamTruthInit_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::TrackParamTruthInit_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::vector<std::string> &input_tags = m_input_tags;
    if(input_tags.empty()) {
        input_tags = GetDefaultInputTags();
    }

    // Get MCParticles
    auto mc_particles = event->Get<edm4hep::MCParticle>(input_tags[0]);

    // Produce track parameters out of MCParticles
    std::vector<Jug::TrackParameters*> results;
    for(auto mc_particle: mc_particles) {

        // Only stable particles from MC
        if(mc_particle->getGeneratorStatus() != 1 ) continue;

        // Do conversion
        auto result = m_truth_track_seeding_algo.produce(mc_particle);
        results.push_back(result);

        // >oO debug output
        if(m_log->level() <= spdlog::level::debug) {
            const auto p = std::hypot(mc_particle->getMomentum().x, mc_particle->getMomentum().y, mc_particle->getMomentum().z);
            const auto charge = result->charge();
            m_log->debug("Invoke track finding seeded by truth particle with:");
            m_log->debug("   p =  {} GeV\"", p);
            m_log->debug("   charge = {}", charge);
            m_log->debug("   q/p =  {}", charge / p);
        }
    }

    Set(results);
}
