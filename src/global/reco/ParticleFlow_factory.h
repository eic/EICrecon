// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) Derek Anderson

#pragma once

#include <map>
#include <spdlog/logger.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/jana/JChainMultifactoryT.h>
// event data model definitions
#include <edm4eic/ReconstructedParticleCollection.h>
// necessary algorithms
#include "algorithms/reco/ParticleFlow.h"
#include "algorithms/reco/ParticleFlowConfig.h"

namespace eicrecon {

  class ParticleFlow_factory :
          public JChainMultifactoryT<ParticleFlowConfig>,
          public SpdlogMixin {

    public:

      // aliases for brevity
      using TrkInput     = const edm4eic::TrackSegmentCollection*;
      using CaloInput    = std::pair<const edm4eic::ClusterCollection*, const edm4eic::ClusterCollection*>;
      using VecCaloInput = std::vector<CaloInput>;

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

      // class-wide constants
      const struct constants {
        size_t nCaloPairs;
        size_t iNegative;
        size_t iCentral;
        size_t iPositive;
      } m_const = {3, 0, 1, 2};

      // map of calo collection tags onto indices
      std::map<std::string, size_t> m_mapCaloInputToIndex = {
        {"EcalEndcapNClusters",     m_const.iNegative},
        {"HcalEndcapNClusters",     m_const.iNegative},
        {"EcalBarrelSciFiClusters", m_const.iCentral},
        {"HcalBarrelClusters",      m_const.iCentral},
        {"EcalEndcapPClusters",     m_const.iPositive},
        {"LFHCALClusters",          m_const.iPositive}
      };

  };  // end ParticleFlow_factory definition

}  // end eicrecon namespace
