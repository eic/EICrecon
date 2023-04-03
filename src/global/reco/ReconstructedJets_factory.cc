// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Zhongling Ji, Derek Anderson

#include "ReconstructedJets_factory.h"

#include <memory>

#include <JANA/JEvent.h>
#include <services/log/Log_service.h>


namespace eicrecon {

    void ReconstructedJets_factory::Init() {

        // This prefix will be used for parameters
        std::string param_prefix = "reco:" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");

        m_jet_algo.init(m_log);
    }

    void ReconstructedJets_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void ReconstructedJets_factory::Process(const std::shared_ptr<const JEvent> &event) {
        auto rc_particles = event->Get<edm4eic::ReconstructedParticle>("ReconstructedParticles");

        std::vector<const edm4hep::LorentzVectorE*> momenta;
        for (const auto& p : rc_particles) {
          // TODO: Need to exclude the scattered electron
          const auto& mom    = p -> getMomentum();
          const auto& energy = p -> getEnergy();
          momenta.push_back(new edm4hep::LorentzVectorE(mom.x, mom.y, mom.z, energy));
        }

        auto jets = m_jet_algo.execute(momenta);
        for (const auso &mom : momenta) {
          delete mom;
        }
        Set(jets);
    }
} // eicrecon
