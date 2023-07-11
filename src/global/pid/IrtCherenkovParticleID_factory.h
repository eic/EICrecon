// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include <extensions/jana/JChainMultifactoryT.h>
#include <JANA/JEvent.h>

// data model
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/CherenkovParticleIDCollection.h>

// algorithms
#include <algorithms/pid/IrtCherenkovParticleID.h>

// services
#include <services/geometry/richgeo/RichGeo_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>

namespace eicrecon {

  class IrtCherenkovParticleID;

  class IrtCherenkovParticleID_factory :
    public JChainMultifactoryT<IrtCherenkovParticleIDConfig>,
    public SpdlogMixin<IrtCherenkovParticleID_factory>
  {

    public:

      explicit IrtCherenkovParticleID_factory(
          std::string tag,
          const std::vector<std::string>& input_tags,
          const std::vector<std::string>& output_tags,
          IrtCherenkovParticleIDConfig cfg
          ):
        JChainMultifactoryT<IrtCherenkovParticleIDConfig>(std::move(tag), input_tags, output_tags, cfg) {
          for(auto& output_tag : GetOutputTags())
            DeclarePodioOutput<edm4eic::CherenkovParticleID>(output_tag);
        }

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void BeginRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

      eicrecon::IrtCherenkovParticleID m_irt_algo;
      std::shared_ptr<RichGeo_service> m_richGeoSvc;
      CherenkovDetectorCollection      *m_irt_det_coll;

  };
}
