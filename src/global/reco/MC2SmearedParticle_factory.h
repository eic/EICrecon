// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <edm4hep/MCParticle.h>
#include <edm4eic/ReconstructedParticle.h>

#include <algorithms/reco/MC2SmearedParticleConfig.h>
#include <algorithms/reco/MC2SmearedParticle.h>
#include "extensions/jana/JChainFactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

    class MC2SmearedParticle_factory:
            public JChainFactoryT<edm4eic::ReconstructedParticle, MC2SmearedParticleConfig>,
            public SpdlogMixin<MC2SmearedParticle_factory> {
    public:

        explicit MC2SmearedParticle_factory(const std::vector<std::string> &default_input_tags, MC2SmearedParticleConfig cfg)
            :JChainFactoryT(default_input_tags, cfg) {}

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        eicrecon::MC2SmearedParticle m_smearing_algo;       /// Actual digitisation algorithm

    };

} // eicrecon

