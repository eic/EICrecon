// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck, Dmitry Kalinkin, Derek Anderson

#pragma once

#include "extensions/jana/JOmniFactory.h"
#include "algorithms/reco/ChargedMCParticleSelector.h"

namespace eicrecon {

  class ChargedMCParticleSelector_factory : public JOmniFactory<ChargedMCParticleSelector_factory, NoConfig> {

    private:

      // algorithm
      std::unique_ptr<eicrecon::ChargedMCParticleSelector> m_algo;

      // input collection
      PodioInput<edm4hep::MCParticle> m_pars_in {this, "GeneratedParticles"};

      // output collection
      PodioOutput<edm4hep::MCParticle> m_pars_out {this};

    public:

      void Configure() {
        m_algo = std::make_unique<eicrecon::ChargedMCParticleSelector>();
        m_algo->init(logger());
      }

      void ChangeRun(int64_t run_number) {
        /* nothing to do */
      }

     void Process(int64_t run_number, int64_t event_number) {
        m_pars_out() = m_algo->process(m_pars_in());
      }

  };

}  // end eicrecon namespace
