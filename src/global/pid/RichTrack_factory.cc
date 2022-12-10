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
  for(int rad=0; rad<rich::nRadiators; rad++) {
    m_trackingPlanes.insert({ rad, m_actsGeo->TrackingPlanes(rad, m_numPlanes[rad]) });
    for(auto plane : m_trackingPlanes.at(rad))
      m_trackingPlanes_all.push_back(plane);
  }

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

    // decide how to build track segments:

    // -- make 1 track segment for all radiators combined
    result.push_back(m_propagation_algo.propagateToSurfaceList( traj, m_trackingPlanes_all ));

    // -- make 1 track segment per radiator
    // for(const auto& [rad,planes] : m_trackingPlanes)
    //   result.push_back(m_propagation_algo.propagateToSurfaceList( traj, planes ));

  } // end trajectory loop

  Set(std::move(result));
}
