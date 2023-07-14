// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Zhongling Ji, Derek Anderson

// standard c includes
#include <memory>
// jana includes
#include <JANA/JEvent.h>
#include <services/log/Log_service.h>
// event data model definitions
#include <edm4hep/MCParticle.h>
// factory-specific includes
#include "GeneratedJets_factory.h"



namespace eicrecon {

    void GeneratedJets_factory::Init() {

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(GetPrefix(), "info");

        // initialize jet reconstruction algorithm
        m_jet_algo.init(m_log);
        return;

    }  // end 'Init()'



    void GeneratedJets_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {

      // nothing to do here
      return;

    }  // end 'BeginRun(std::shared_ptr<JEvent&>)'



    void GeneratedJets_factory::Process(const std::shared_ptr<const JEvent> &event) {

        // loop over input tags
        size_t iInput = 0;
        for (const std::string& input_tag : GetInputTags()) {  

          // grab input collection
          auto input = event -> Get<edm4hep::MCParticle>(input_tag);

          // extract particle momenta
          std::vector<const edm4hep::LorentzVectorE*> momenta;
          for (const auto& particle : input) {

            // select only final state particles
            const bool is_final_state = (particle -> getGeneratorStatus() == 1);
            if (!is_final_state) continue;

            const auto& momentum = particle -> getMomentum();
            const auto& energy   = particle -> getEnergy();
            momenta.push_back(new edm4hep::LorentzVectorE(momentum.x, momentum.y, momentum.z, energy));
          }  // end particle loop

          // run algorithm
          auto gen_jets = m_jet_algo.execute(momenta);
          for (const auto &momentum : momenta) {
            delete momentum;
          }

          // set output collection
          SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[iInput], std::unique_ptr<edm4eic::ReconstructedParticleCollection>(gen_jets));
          ++iInput;

        }  // end input tag loop
        return;

    }  // end 'Process(shared_ptr<JEvent>)'

}  // end eicrecon namespace
