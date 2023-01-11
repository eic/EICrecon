// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <vector>

#include "TrackParamTruthInit_factory.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "extensions/string/StringHelpers.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>
#include <JANA/JEvent.h>

#include <algorithms/tracking/TrackParamTruthInit.h>
#include <algorithms/tracking/TrackParamTruthInitConfig.h>

void eicrecon::TrackParamTruthInit_factory::Init() {
    auto app = GetApplication();

    // This prefix will be used for parameters
    std::string param_prefix = GetDefaultParameterPrefix();

    InitLogger(param_prefix);
    InitDataTags(param_prefix);

    // Algorithm configuration
    m_truth_track_seeding_algo.m_momentum_split = 0.0;
    m_truth_track_seeding_algo.m_momentum_smear = 0.0;
    app->SetDefaultParameter(param_prefix + ":TruthSeeding:MomentumSplit", m_truth_track_seeding_algo.m_momentum_split, "Momentum magnitude fraction to use as width for random trifurcation");
    app->SetDefaultParameter(param_prefix + ":TruthSeeding:MomentumSmear", m_truth_track_seeding_algo.m_momentum_smear, "Momentum magnitude fraction to use as width of gaussian smearing");

    // Initialize underlying algorithm
    m_truth_track_seeding_algo.init(m_log);
}

void eicrecon::TrackParamTruthInit_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::TrackParamTruthInit_factory::Process(const std::shared_ptr<const JEvent> &event) {

    // Get MCParticles
    auto mc_particles = event->Get<edm4hep::MCParticle>(GetInputTags()[0]);

    try {
        // Produce track parameters out of MCParticles
        std::vector<eicrecon::TrackParameters *> results;
        for (auto mc_particle: mc_particles) {

            // Only stable particles from MC
            if (mc_particle->getGeneratorStatus() != 1) continue;

            // Do conversion
            auto result = m_truth_track_seeding_algo.produce(mc_particle);

            if (!result) continue;   // result might be null

            results.push_back(result);

            // >oO debug output
            if (m_log->level() <= spdlog::level::debug) {
                const auto p = std::hypot(mc_particle->getMomentum().x, mc_particle->getMomentum().y,
                                          mc_particle->getMomentum().z);
                const auto charge = result->charge();
                m_log->debug("Invoke track finding seeded by truth particle with:");
                m_log->debug("   p =  {} GeV\"", p);
                m_log->debug("   charge = {}", charge);
                m_log->debug("   q/p =  {}", charge / p);
            }
        }
        Set(results);
    }
    catch(std::exception &e) {
        m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
    }
}
