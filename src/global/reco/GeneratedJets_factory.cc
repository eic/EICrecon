// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Zhongling Ji, Derek Anderson

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <Math/GenVector/LorentzVector.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/kinematics.h>
#include <memory>

// factory-specific includes
#include "GeneratedJets_factory.h"
#include "datamodel_glue.h"

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

        // extract particle momenta
        std::vector<const edm4hep::LorentzVectorE*> momenta;
        for (const auto& particle : *input) {

            // select only final state charged particles
            const bool is_final_state = (particle.getGeneratorStatus() == 1);
            if (!is_final_state) continue;

            const auto& momentum = particle.getMomentum();
            const auto& energy = particle.getEnergy();
            momenta.push_back(new edm4hep::LorentzVectorE(momentum.x, momentum.y, momentum.z, energy));
        }  // end particle loop

        // run algorithm
        auto gen_jets = m_jet_algo.process(momenta);
        for (const auto &momentum : momenta) {
            delete momentum;
        }

        // set output collection
        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::move(gen_jets));

    }  // end 'Process(shared_ptr<JEvent>)'

}  // end eicrecon namespace
