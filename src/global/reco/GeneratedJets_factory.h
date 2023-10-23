// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Zhongling Ji, Derek Anderson

#pragma once

#include <JANA/JEvent.h>
// event data model definitions
#include <edm4eic/ReconstructedParticleCollection.h>
#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

// necessary algorithms
#include "algorithms/reco/JetReconstruction.h"
#include "algorithms/reco/JetReconstructionConfig.h"

namespace eicrecon {

    class GeneratedJets_factory :
            public JChainMultifactoryT<JetReconstructionConfig>,
            public SpdlogMixin {

    public:

        // ctor
        explicit GeneratedJets_factory(std::string tag,
                                       const std::vector<std::string>& input_tags,
                                       const std::vector<std::string>& output_tags,
                                       JetReconstructionConfig cfg) :
                 JChainMultifactoryT<JetReconstructionConfig>(std::move(tag), input_tags, output_tags, cfg) {

            DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags()[0]);
        }  // end ctor

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void BeginRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    protected:

        JetReconstruction m_jet_algo;

  };  // end GeneratedJets_factory definition

}  // end eicrecon namespace
