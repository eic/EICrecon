// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include <extensions/jana/JChainFactoryT.h>
#include <JANA/JEvent.h>

// data model
#include <edm4eic/TrackSegmentCollection.h>

// algorithms
#include <algorithms/tracking/TrackPropagation.h>

// services
#include <services/geometry/richgeo/RichGeo_service.h>
#include <services/geometry/acts/ACTSGeo_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

namespace eicrecon {
  class RichTrack_factory :
    public JChainFactoryT<edm4eic::TrackSegment>,
    public SpdlogMixin<RichTrack_factory>
  {
    public:

      explicit RichTrack_factory(std::vector<std::string> default_input_tags) :
          JChainFactoryT<edm4eic::TrackSegment>(std::move(default_input_tags)) {}

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
      std::string m_detector_name;
      std::shared_ptr<RichGeo_service> m_richGeoSvc;
      std::shared_ptr<ACTSGeo_service> m_actsSvc;
      richgeo::ActsGeo *m_actsGeo;
      std::vector<std::shared_ptr<Acts::Surface>> m_trackingPlanes;
      int m_numPlanes;
      int m_radiatorID;
      eicrecon::TrackPropagation m_propagation_algo;
  };
}
