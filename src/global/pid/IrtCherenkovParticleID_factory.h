// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <IRT/CherenkovDetectorCollection.h>
#include <JANA/JEvent.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// algorithms
#include "algorithms/pid/IrtCherenkovParticleID.h"
#include "algorithms/pid/IrtCherenkovParticleIDConfig.h"
// JANA
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"
// services
#include "services/geometry/richgeo/RichGeo_service.h"

namespace eicrecon {

  class IrtCherenkovParticleID_factory :
    public JChainMultifactoryT<IrtCherenkovParticleIDConfig>,
    public SpdlogMixin
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
