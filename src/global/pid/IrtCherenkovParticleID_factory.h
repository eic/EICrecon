// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include <extensions/jana/JChainFactoryT.h>
#include <JANA/JEvent.h>

// data model
#include <edm4eic/RawPMTHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/CherenkovParticleIDCollection.h>

// algorithms
#include <algorithms/pid/IrtCherenkovParticleID.h>

// services
#include <services/geometry/richgeo/RichGeo_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

namespace eicrecon {
  class IrtCherenkovParticleID;
  class IrtCherenkovParticleID_factory :
    public JChainFactoryT<edm4eic::CherenkovParticleID, IrtCherenkovParticleIDConfig>,
    public SpdlogMixin<IrtCherenkovParticleID_factory>
  {

    public:

      explicit IrtCherenkovParticleID_factory(std::vector<std::string> default_input_tags, IrtCherenkovParticleIDConfig cfg) :
        JChainFactoryT<edm4eic::CherenkovParticleID, IrtCherenkovParticleIDConfig>(std::move(default_input_tags), cfg) {}

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
      eicrecon::IrtCherenkovParticleID m_irt_algo;
      std::string                      m_detector_name;
      std::shared_ptr<RichGeo_service> m_richGeoSvc;
      CherenkovDetectorCollection      *m_irt_det_coll;

  };
}
