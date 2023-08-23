// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Zhongling Ji, Derek Anderson

// standard c includes
#include <memory>
#include <JANA/JEvent.h>
#include <services/log/Log_service.h>
// event data model definitions
#include <edm4eic/ReconstructedParticle.h>
// factory-specific includes
#include "ReconstructedJets_factory.h"



namespace eicrecon {

    void ReconstructedJets_factory::Init() {

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(GetApplication(), GetPrefix(), "info");

        // initialize jet reconstruction algorithm
        m_jet_algo.init(m_log);

    }  // end 'Init()'



    void ReconstructedJets_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {

        // Nothing to do here

    }  // end 'BeginRun(std::shared_ptr<JEvent&>)'



    void ReconstructedJets_factory::Process(const std::shared_ptr<const JEvent> &event) {

        // grab input collection
        auto input = event->Get<edm4eic::ReconstructedParticle>(GetInputTags()[0]);

        // extract particle momenta
        std::vector<const edm4hep::LorentzVectorE*> momenta;
        for (const auto& particle : input) {

            // TODO: Need to exclude the scattered electron
            const auto& momentum = particle->getMomentum();
            const auto& energy = particle->getEnergy();
            momenta.push_back(new edm4hep::LorentzVectorE(momentum.x, momentum.y, momentum.z, energy));
        }  // end particle loop

        // run algorithm
        auto rec_jets = m_jet_algo.execute(momenta);
        for (const auto &momentum : momenta) {
            delete momentum;
        }

        // set output collection
        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::move(rec_jets));

    }  // end 'Process(shared_ptr<JEvent>)'

}  // eicrecon namespace
