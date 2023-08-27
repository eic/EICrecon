// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Zhongling Ji, Derek Anderson

#pragma once

#include <spdlog/logger.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/jana/JChainMultifactoryT.h>
// event data model definitions
#include <edm4eic/ReconstructedParticleCollection.h>
// necessary algorithms
#include "algorithms/reco/JetReconstruction.h"



namespace eicrecon {

    class ReconstructedJets_factory :
            public JChainMultifactoryT<NoConfig>,
            public SpdlogMixin {

    public:

        // ctor
        explicit ReconstructedJets_factory(const std::vector<std::string>& input_tags,
                                           const std::vector<std::string>& output_tags) :
                 JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {

            DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags()[0]);
        }  // end ctor

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void BeginRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    protected:

        JetReconstruction        m_jet_algo;

    };  // end ReconstructedJets_factory definition

}  // end eicrecon namespace
