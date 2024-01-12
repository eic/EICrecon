/* COPYRIGHT STUFF */

#pragma once

#include "extensions/jana/JOmniFactory.h"
#include "algorithms/reco/ChargedReconstructedParticleSelector.h"

namespace eicrecon {

  class ChargedReconstructedParticleSelector_factory : public JOmniFactory<ChargedReconstructedParticleSelector_factory, NoConfig> {

    private:

      // algorithm
      std::unique_ptr<eicrecon::ChargedReconstructedParticleSelector> m_algo;

      // input collection
      PodioInput<edm4eic::ReconstructedParticle> m_pars_in {this, "GeneratedParticles"};

      // output collection
      PodioOutput<edm4eic::ReconstructedParticle> m_pars_out {this};

    public:

      void Configure() {
        m_algo = std::make_unique<eicrecon::ChargedReconstructedParticleSelector>();
        m_algo -> init(logger());
      }

      void ChangeRun(int64_t run_number) {
        /* nothing to do */
      }

     void Process(int64_t run_number, int64_t event_number) {
        m_pars_out() = m_algo -> process(m_pars_in());
      }

  };

}  // end eicrecon namespace
