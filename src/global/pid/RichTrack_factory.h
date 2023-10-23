// Copyright (C) 2022, 2023 Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <Acts/Surfaces/Surface.hpp>
#include <JANA/JEvent.h>
#include <edm4eic/TrackPoint.h>
// data model
#include <edm4eic/TrackSegmentCollection.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// algorithms
#include "algorithms/tracking/TrackPropagation.h"
// JANA
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"
// configuration
#include "global/pid/RichTrackConfig.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/richgeo/ActsGeo.h"
// services
#include "services/geometry/richgeo/RichGeo_service.h"

namespace eicrecon {
  class RichTrack_factory :
    public JChainMultifactoryT<RichTrackConfig>,
    public SpdlogMixin
  {
    public:

      explicit RichTrack_factory(
          std::string tag,
          const std::vector<std::string>& input_tags,
          const std::vector<std::string>& output_tags,
          RichTrackConfig cfg
          ):
        JChainMultifactoryT<RichTrackConfig>(std::move(tag), input_tags, output_tags, cfg) {
          for(auto& output_tag : GetOutputTags())
            DeclarePodioOutput<edm4eic::TrackSegment>(output_tag);
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
      // map: output tag name -> cuts
      std::map< std::string, std::function<bool(edm4eic::TrackPoint)> > m_track_point_cuts;


      // underlying algorithm
      eicrecon::TrackPropagation m_propagation_algo;
  };
}
