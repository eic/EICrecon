// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#include "TruthTrackSeeding_factory.h"

void eicrecon::TruthTrackSeeding_factory::Init() {
    // This prefix will be used for parameters
    std::string param_prefix = "TrkHitReco_" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

    // Create plugin level sub-log
    m_log = spdlog::stdout_color_mt("TrackerSourceLinker_factory");

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

    // Get ACTS context from ACTSGeo service
    auto acts_service = GetApplication()->GetService<ACTSGeo_service>();
    m_acts_context = acts_service->acts_context();

    auto dd4hp_service = GetApplication()->GetService<JDD4hep_service>();

    // Initialize algorithm
    auto cellid_converter = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*dd4hp_service->detector());
    m_source_linker.init(cellid_converter, m_log);
}

void eicrecon::TruthTrackSeeding_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::TruthTrackSeeding_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::vector<std::string> &input_tags = m_input_tags;
    if(input_tags.size() == 0) {
        input_tags = GetDefaultInputTags();
    }

    // Collect all hits
    std::vector<const eicd::TrackParameters *> total_sim_hits;
    for(auto input_tag: input_tags) {
        auto simHits = event->Get<edm4hep::MCParticle>(input_tag);
        for (const auto ahit : simHits) {
            total_sim_hits.push_back(ahit);                     /// TODO a better way to concatenate arrays
        }
    }

}
