// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) Derek Anderson

#pragma once

#include <map>
#include <vector>
#include <utility>
#include <spdlog/logger.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/jana/JChainMultifactoryT.h>
// event data model definitions
#include <edm4eic/ReconstructedParticleCollection.h>
// for geometry services
#include <services/geometry/dd4hep/DD4hep_service.h>
// necessary algorithms and configurations
#include "algorithms/reco/ParticleFlow.h"
#include "algorithms/reco/ParticleFlowConfig.h"

namespace eicrecon {

  class ParticleFlow_factory :
          public JChainMultifactoryT<ParticleFlowConfig>,
          public SpdlogMixin {

    public:

      // aliases for brevity
      using TrkInput     = std::pair<const edm4eic::ReconstructedParticleCollection*, const edm4eic::TrackSegmentCollection*>;
      using CaloInput    = std::pair<const edm4eic::ClusterCollection*, const edm4eic::ClusterCollection*>;
      using CaloIDs      = std::pair<uint32_t, uint32_t>;
      using VecCaloInput = std::vector<CaloInput>;
      using VecCaloIDs   = std::vector<CaloIDs>;

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

      // particle flow algorithm
      ParticleFlow m_pf_algo;

      // geometry service
      std::shared_ptr<DD4hep_service> m_geoSvc;

      // class-wide constants
      const struct constants {
        size_t nCaloPairs;
        size_t iNegative;
        size_t iCentral;
        size_t iPositive;
      } m_const = {3, 0, 1, 2};

      // map of calo collection tags, id names onto indices
      std::map<std::string, size_t> m_mapCaloInputToIndex = {
        {"EcalEndcapNClusters",    m_const.iNegative},
        {"HcalEndcapNClusters",    m_const.iNegative},
        {"EcalBarrelScFiClusters", m_const.iCentral},
        {"HcalBarrelClusters",     m_const.iCentral},
        {"EcalEndcapPClusters",    m_const.iPositive},
        {"LFHCALClusters",         m_const.iPositive}
      };
      std::map<size_t, std::string> m_mapIndexToECalID = {
        {m_const.iNegative, "ECalEndcapN_ID"},
        {m_const.iCentral,  "ECalBarrel_ID"},
        {m_const.iPositive, "ECalEndcapP_ID"}
      };
      std::map<size_t, std::string> m_mapIndexToHCalID = {
        {m_const.iNegative, "HCalEndcapN_ID"},
        {m_const.iCentral,  "HCalBarrel_ID"},
        {m_const.iPositive, "HCalEndcapP_ID"}
      };

  };  // end ParticleFlow_factory definition

}  // end eicrecon namespace
