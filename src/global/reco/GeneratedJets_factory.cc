// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Zhongling Ji, Derek Anderson

// standard c includes
#include <memory>
#include <JANA/JEvent.h>
// event data model definitions
#include <edm4hep/MCParticleCollection.h>
// factory-specific includes
#include "GeneratedJets_factory.h"

namespace eicrecon {

    void GeneratedJets_factory::Init() {

        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string plugin_name = GetPluginName();
        std::string param_prefix = plugin_name + ":" + GetTag();

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(GetApplication(), GetPrefix(), "info");

        // Algorithm configuration
        auto cfg = GetDefaultConfig();

        app->SetDefaultParameter(param_prefix + ":minCstPt",       cfg.minCstPt);
        app->SetDefaultParameter(param_prefix + ":maxCstPt",       cfg.maxCstPt);
        app->SetDefaultParameter(param_prefix + ":rJet",           cfg.rJet);
        app->SetDefaultParameter(param_prefix + ":minJetPt",       cfg.minJetPt);
        app->SetDefaultParameter(param_prefix + ":numGhostRepeat", cfg.numGhostRepeat);
        app->SetDefaultParameter(param_prefix + ":ghostMaxRap",    cfg.ghostMaxRap);
        app->SetDefaultParameter(param_prefix + ":ghostArea",      cfg.ghostArea);
        app->SetDefaultParameter(param_prefix + ":jetAlgo",        cfg.jetAlgo);
        app->SetDefaultParameter(param_prefix + ":recombScheme",   cfg.recombScheme);
        app->SetDefaultParameter(param_prefix + ":areaType",       cfg.areaType);

        // initialize jet reconstruction algorithm
        m_jet_algo.applyConfig(cfg);
        m_jet_algo.init(logger());

    }  // end 'Init()'



    void GeneratedJets_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {

      // nothing to do here

    }  // end 'BeginRun(std::shared_ptr<JEvent&>)'



    void GeneratedJets_factory::Process(const std::shared_ptr<const JEvent> &event) {

        // grab input collection
        auto input = static_cast<const edm4hep::MCParticleCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        // select only final state particles for reconstruction
        // TODO: Need to exclude the scattered electron
        std::unique_ptr<edm4hep::MCParticleCollection> for_reconstruction = std::make_unique<edm4hep::MCParticleCollection>();
        for (const auto& particle : *input) {
            const bool is_final_state = (particle.getGeneratorStatus() == 1);
            if (is_final_state) {
                for_reconstruction -> push_back(particle);
            }
        }

        // run algorithm
        auto gen_jets = m_jet_algo.process(std::move(for_reconstruction));

        // set output collection
        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::move(gen_jets));

    }  // end 'Process(shared_ptr<JEvent>)'

}  // end eicrecon namespace
