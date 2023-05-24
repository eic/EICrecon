// Copyright (C) 2022, 2023 Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include <extensions/jana/JChainMultifactoryT.h>
#include <JANA/JEvent.h>

// data model
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/TrackCollection.h>

// algorithms
#include <algorithms/tracking/TrackPropagation.h>

// configuration
#include <global/pid/RichTrackConfig.h>

// services
#include <services/geometry/richgeo/RichGeo_service.h>
#include <services/geometry/acts/ACTSGeo_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

namespace eicrecon {
  class RichTrack_factory :
    public JChainMultifactoryT<RichTrackConfig>,
    public SpdlogMixin<RichTrack_factory>
  {
    public:

      explicit RichTrack_factory(
          std::string tag,
          const std::vector<std::string>& input_tags,
          const std::vector<std::string>& output_tags,
          RichTrackConfig cfg
          ):
        JChainMultifactoryT<RichTrackConfig>(std::move(tag), input_tags, output_tags, cfg) {
          for(auto& output_tag : GetOutputTags()) {
            if(output_tag.find("TrackID") == std::string::npos)
              DeclarePodioOutput<edm4eic::TrackSegment>(output_tag);
            else
              DeclarePodioOutput<edm4eic::Track>(output_tag);
          }
        }

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void BeginRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
      std::string m_detector_name;
      std::shared_ptr<RichGeo_service> m_richGeoSvc;
      std::shared_ptr<ACTSGeo_service> m_actsSvc;
      richgeo::ActsGeo *m_actsGeo;

      // map: output_tag name (for a radiator's track projections) -> a vector of xy-planes to project to
      std::map< std::string, std::vector<std::shared_ptr<Acts::Surface>> > m_tracking_planes;

      // name of trackIDs output collection
      std::string m_trackIDs_tag;

      // underlying algorithm
      eicrecon::TrackPropagation m_propagation_algo;
  };
}
