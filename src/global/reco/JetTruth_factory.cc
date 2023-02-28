// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson, Zhongling Ji

#include <memory>

#include <JANA/JEvent.h>

#include <spdlog/spdlog.h>

#include "JetTruth_factory.h"

#include <edm4hep/MCParticle.h>
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "algorithms/reco/JetReconstruction.h"

namespace eicrecon {

    void JetTruth_factory::Init() {

        // This prefix will be used for parameters
        std::string param_prefix = "reco:" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");

        m_inclusive_kinematics_algo.init(m_log);
    }

    void JetTruth_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void JetTruth_factory::Process(const std::shared_ptr<const JEvent> &event) {
        auto mc_particles = event->Get<edm4hep::MCParticle>("MCParticles");

        std::vector<edm4hep::Vector3f> momenta;
        for (const auto& p: mc_particles)
          if (p->getGeneratorStatus() == 1)
            momenta.push_back(p->getMomentum());

        auto jets = m_jet_algo.execute(momenta);
        Set(jets);
    }
} // eicrecon
