// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "RichTrack_factory.h"

#include <edm4eic/vector_utils.h>

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::Init() {
  auto app = GetApplication();

  // input tags
  auto m_detector_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
  auto param_prefix = m_detector_name + ":" + GetTag();
  InitDataTags(param_prefix);

  // services
  m_richGeoSvc = app->GetService<RichGeo_service>();
  m_actsSvc    = app->GetService<ACTSGeo_service>();
  InitLogger(param_prefix, "info");
  m_propagation_algo.init(m_actsSvc->actsGeoProvider(), m_log);
  m_log->debug("m_detector_name='{}'  param_prefix='{}'", m_detector_name, param_prefix);

  // default configuration parameters
  m_numPlanes[rich::kAerogel] = 5;
  m_numPlanes[rich::kGas]     = 10;

  // configuration parameters
  auto set_param = [&param_prefix, &app, this] (std::string name, auto &val, std::string description) {
    app->SetDefaultParameter(param_prefix+":"+name, val, description);
  };
  m_log->debug("m_numPlanes:");
  for(int rad=0; rad<rich::nRadiators; rad++) {
    auto radName = rich::RadiatorName(rad);
    set_param(
        "numPlanes:"+radName,
        m_numPlanes[rad],
        "number of "+radName+" radiator track-projection planes"
        );
    m_log->debug("  {:>10}: {}", rich::RadiatorName(rad), m_numPlanes[rad]);
  }
  
  // get RICH geometry for track projection
  m_actsGeo = m_richGeoSvc->GetActsGeo(m_detector_name);
  for(int rad=0; rad<rich::nRadiators; rad++)
    m_trackingPlanes.insert({ rad, m_actsGeo->TrackingPlanes(rad, m_numPlanes[rad]) });
}

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
}

//-----------------------------------------------------------------------------
void eicrecon::RichTrack_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // collect all trajectories all input tags
  std::vector<const eicrecon::TrackingResultTrajectory*> trajectories;
  for(const auto& input_tag : GetInputTags()) {
    try {
      for(const auto traj : event->Get<eicrecon::TrackingResultTrajectory>(input_tag))
        trajectories.push_back(traj);
    } catch(std::exception &e) {
      m_log->critical(e.what());
      throw JException(e.what());
    }
  }

  // result will be `TrackSegments`s for each trajectory
  std::vector<edm4eic::TrackSegment*> result;

  // loop over trajectories
  m_log->trace("Propagate trajectories: --------------------");
  for(const auto& traj : trajectories) { 

    // start a new `TrackSegment`, which will be composed of `TrackPoints` that have been
    // projected from `traj` onto the Discs
    edm4eic::MutableTrackSegment track_segment;
    decltype(edm4eic::TrackSegmentData::length)      length       = 0;
    decltype(edm4eic::TrackSegmentData::lengthError) length_error = 0;

    // loop over radiators
    m_log->trace("<><><> Trajectory:");
    for(const auto& [rad,planes] : m_trackingPlanes) {
      m_log->trace("<> radiator = {}", rich::RadiatorName(rad));

      // loop over track-projection planes for this radiator
      for(const auto& plane : planes) {

        // get the `TrackPoint` by projecting `traj` to this `plane`
        edm4eic::TrackPoint *point;
        try {
          point = m_propagation_algo.propagate(traj, plane);
        } catch(std::exception &e) {
          m_log->warn("<> Exception in underlying algorithm: {}; skip this TrackPoint", e.what());
        }
        if(point) {

          // logging
          m_log->trace("<> trajectory: x=( {:>10.2f} {:>10.2f} {:>10.2f} )",
              point->position.x, point->position.y, point->position.z);
          m_log->trace("               p=( {:>10.2f} {:>10.2f} {:>10.2f} )",
              point->momentum.x, point->momentum.y, point->momentum.z);

          // update the `TrackSegment` length
          if(track_segment.points_size()>0) {
            auto pos0 = point->position;
            auto pos1 = std::prev(track_segment.points_end())->position;
            auto dist = edm4eic::magnitude(pos0-pos1);
            length += dist;
            m_log->trace("               dist to previous point: {}", dist);
          }

          // add the `TrackPoint` to the `TrackSegment`
          track_segment.addToPoints(*point);
        }
        else m_log->trace("<> Failed to propagate trajectory to this plane");

      } // end plane loop
    } // end radiator loop

    // add this `TrackSegment` to the output
    track_segment.setLength(length);
    track_segment.setLengthError(length_error);
    result.push_back(new edm4eic::TrackSegment(track_segment));

  } // end trajectory loop

  Set(std::move(result));
}
