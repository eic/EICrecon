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
        InitLogger(app, param_prefix, "info");

        // Initialize digitization algorithm
        m_smearing_algo.init(m_log);
    }

    void MC2SmearedParticle_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

    }

    void MC2SmearedParticle_factory::Process(const std::shared_ptr<const JEvent> &event) {
        // Collect all hits from different tags
        const auto mc_particles = static_cast<const edm4hep::MCParticleCollection*>(event->GetCollectionBase(GetInputTags()[0]));
        auto reco_particles = m_smearing_algo.produce(mc_particles);

        // Set the result
        SetCollection(std::move(reco_particles));     // Add data as a factory output
    }
} // eicrecon
