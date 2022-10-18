// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#include <JANA/JEvent.h>
#include "MC2SmearedParticle_factory.h"

namespace eicrecon {
    void MC2SmearedParticle_factory::Init() {
        auto app = GetApplication();
        // We will use plugin name to get parameters for correct factory
        // So if we use <plugin name>:parameter whichever plugin uses this template. eg:
        //    "BTRK:parameter" or "FarForward:parameter"
        // That has limitations but the convenient in the most of the cases
        std::string param_prefix = "Reco:" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

        // Set input tags
        InitDataTags(param_prefix);

        // Logger. Get plugin level sub-log
        InitLogger(param_prefix, "info");

        // Setup digitization algorithm
        auto cfg = GetDefaultConfig();
        app->SetDefaultParameter(param_prefix + ":MomentumSmearing", cfg.momentum_smearing, "Gaussian momentum smearing value");

        // Initialize digitization algorithm
        m_smearing_algo.applyConfig(cfg);
        m_smearing_algo.init(m_log);
    }

    void MC2SmearedParticle_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

    }

    void MC2SmearedParticle_factory::Process(const std::shared_ptr<const JEvent> &event) {
        // Collect all hits from different tags
        std::vector<edm4eic::ReconstructedParticle *> reco_particles;           // output collection
        const auto &input_tag = GetInputTags()[0];               // Name of the collection to read
        auto mc_particles = event->Get<edm4hep::MCParticle>(input_tag);

        for (const auto mc_particle : mc_particles) {
            auto reco_particle = m_smearing_algo.produce(mc_particle);
            reco_particles.push_back(reco_particle);
        }

        // Set the result
        Set(std::move(reco_particles));     // Add data as a factory output
    }
} // eicrecon