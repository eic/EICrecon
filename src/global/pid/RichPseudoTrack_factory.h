// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include <extensions/jana/JChainFactoryT.h>
#include <JANA/JEvent.h>

// data model
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>

// algorithms
#include <algorithms/pid/PseudoTracks.h>

// services
#include <services/geometry/richgeo/RichGeo_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

namespace eicrecon {
  class RichPseudoTrack_factory :
    public JChainFactoryT<edm4eic::TrackSegment, PseudoTracksConfig>,
    public SpdlogMixin<RichPseudoTrack_factory>
  {
    public:

      explicit RichPseudoTrack_factory(std::vector<std::string> default_input_tags, PseudoTracksConfig cfg) :
          JChainFactoryT<edm4eic::TrackSegment, PseudoTracksConfig>(std::move(default_input_tags), cfg) {}

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
      std::string m_detector_name;
      std::shared_ptr<RichGeo_service> m_richGeoSvc;
      richgeo::ActsGeo *m_actsGeo;
      int m_numPlanes;
      int m_radiatorID;
      eicrecon::PseudoTracks m_tracks_algo;
  };
}
