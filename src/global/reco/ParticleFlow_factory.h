// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) Derek Anderson

#pragma once

#include <spdlog/logger.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/jana/JChainMultifactoryT.h>
// event data model definitions
#include <edm4eic/ReconstructedParticleCollection.h>
// necessary algorithms
#include "algorithms/reco/ParticleFlow.h"

namespace eicrecon {

  class ParticleFlow_factory :
          public JChainMultifactoryT<ParticleFlowConfig>,
           public SpdlogMixin {

    public:

      // ctor
      explicit ParticleFlow_factory(std::string tag,
                                         const std::vector<std::string>& input_tags,
                                         const std::vector<std::string>& output_tags,
                                         ParticleFlowConfig cfg) :
               JChainMultifactoryT<ParticleFlowConfig>(std::move(tag), input_tags, output_tags, cfg) {

          DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags()[0]);
      }  // end ctor

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void BeginRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    protected:

      ParticleFlow m_pf_algo;

  };  // end ParticleFlow_factory definition

}  // end eicrecon namespace
